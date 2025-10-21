/* 
 * File:   GPS_I2C.c
 * Author: EQUIPO
 *
 * Created on 20 de octubre de 2025, 05:03 PM
 */

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ================== CONFIGURACIÓN ==================
#pragma config FOSC = INTOSCIO_EC // Oscilador interno, RA6/RA7 como I/O
#pragma config WDT = OFF          // Watchdog Timer desactivado
#pragma config LVP = OFF          // Programación baja tensión desactivada
#pragma config PBADEN = OFF       // PORTB digital al reset
#pragma config MCLRE = ON         // MCLR habilitado

#define _XTAL_FREQ 8000000UL      // Frecuencia del oscilador interno (8 MHz)

// ================== LCD 16x2 ==================
#define RS LATBbits.LATB0
#define EN LATBbits.LATB1
#define D4 LATBbits.LATB2
#define D5 LATBbits.LATB3
#define D6 LATBbits.LATB4
#define D7 LATBbits.LATB5

// ================== FUNCIONES LCD ==================
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_Init(void);
void LCD_String(const char *msg);
void LCD_Clear(void);
void LCD_Set_Cursor(unsigned char row, unsigned char col);

// ================== UART ==================
void UART_Init(void);
void UART_Write(char data);
char UART_Read(void);
unsigned char UART_Data_Ready(void);

// ================== GPS ==================
#define NMEA_BUF_SIZE 100
char nmea_buffer[NMEA_BUF_SIZE];
int nmea_index = 0;

// ================== FUNCIONES GPS ==================
void parse_nmea_line(const char *line);
double nmea_latlon_to_decimal(const char *nmea, char hemi);
void display_latlon(double lat, double lon);

// ================== MAIN ==================
void main(void)
{
    // Configuración del oscilador interno a 8MHz
    OSCCON = 0b01110010; // 8 MHz, estable

    TRISB = 0x00;  // Puerto B como salida (LCD)
    LATB = 0x00;

    LCD_Init();
    UART_Init();

    LCD_Clear();
    LCD_String("Esperando GPS...");

    while (1)
    {
        if (UART_Data_Ready())
        {
            char c = UART_Read();

            if (c == '\n')
            {
                nmea_buffer[nmea_index] = '\0';
                parse_nmea_line(nmea_buffer);
                nmea_index = 0;
            }
            else if (nmea_index < NMEA_BUF_SIZE - 1)
            {
                nmea_buffer[nmea_index++] = c;
            }
        }
    }
}

// ================== LCD FUNCIONES ==================
void LCD_Command(unsigned char cmd)
{
    RS = 0;
    LATB = (LATB & 0xC3) | ((cmd >> 2) & 0x3C);
    EN = 1;
    __delay_ms(2);
    EN = 0;
    LATB = (LATB & 0xC3) | ((cmd << 2) & 0x3C);
    EN = 1;
    __delay_ms(2);
    EN = 0;
}

void LCD_Char(unsigned char data)
{
    RS = 1;
    LATB = (LATB & 0xC3) | ((data >> 2) & 0x3C);
    EN = 1;
    __delay_ms(2);
    EN = 0;
    LATB = (LATB & 0xC3) | ((data << 2) & 0x3C);
    EN = 1;
    __delay_ms(2);
    EN = 0;
}

void LCD_Init(void)
{
    __delay_ms(20);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
}

void LCD_String(const char *msg)
{
    while (*msg)
        LCD_Char(*msg++);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    __delay_ms(2);
}

void LCD_Set_Cursor(unsigned char row, unsigned char col)
{
    unsigned char pos;
    if (row == 1)
        pos = 0x80 + (col - 1);
    else
        pos = 0xC0 + (col - 1);
    LCD_Command(pos);
}

// ================== UART FUNCIONES ==================
void UART_Init(void)
{
    TRISCbits.TRISC6 = 0; // TX (pin 26)
    TRISCbits.TRISC7 = 1; // RX (pin 25)

    SPBRG = 51;           // 9600 baudios con 8 MHz
    TXSTAbits.BRGH = 1;   // Alta velocidad
    RCSTAbits.SPEN = 1;   // Habilitar puerto serial
    TXSTAbits.TXEN = 1;   // Habilitar transmisión
    RCSTAbits.CREN = 1;   // Habilitar recepción
}

void UART_Write(char data)
{
    while (!TXSTAbits.TRMT);
    TXREG = data;
}

char UART_Read(void)
{
    while (!PIR1bits.RCIF);
    return RCREG;
}

unsigned char UART_Data_Ready(void)
{
    return PIR1bits.RCIF;
}

// ================== GPS FUNCIONES ==================
void parse_nmea_line(const char *line)
{
    if (strncmp(line, "$GPRMC", 6) == 0 || strncmp(line, "$GPGGA", 6) == 0)
    {
        char tmp[NMEA_BUF_SIZE];
        strncpy(tmp, line, NMEA_BUF_SIZE - 1);
        tmp[NMEA_BUF_SIZE - 1] = '\0';

        char *token;
        int field = 0;
        char lat_field[20] = "";
        char lon_field[20] = "";
        char lat_hemi = 'N';
        char lon_hemi = 'E';

        token = strtok(tmp, ",");
        while (token != NULL)
        {
            field++;

            if (strncmp(line, "$GPRMC", 6) == 0)
            {
                if (field == 3)
                    strcpy(lat_field, token);
                else if (field == 4)
                    lat_hemi = token[0];
                else if (field == 5)
                    strcpy(lon_field, token);
                else if (field == 6)
                    lon_hemi = token[0];
            }
            else
            { // GPGGA
                if (field == 2)
                    strcpy(lat_field, token);
                else if (field == 3)
                    lat_hemi = token[0];
                else if (field == 4)
                    strcpy(lon_field, token);
                else if (field == 5)
                    lon_hemi = token[0];
            }

            token = strtok(NULL, ",");
        }

        if (strlen(lat_field) && strlen(lon_field))
        {
            double lat = nmea_latlon_to_decimal(lat_field, lat_hemi);
            double lon = nmea_latlon_to_decimal(lon_field, lon_hemi);
            display_latlon(lat, lon);
        }
    }
}

double nmea_latlon_to_decimal(const char *nmea, char hemi)
{
    int deg_digits = (strlen(nmea) > 5) ? 2 : 3;
    char deg_str[4] = {0};
    strncpy(deg_str, nmea, (size_t)deg_digits);

    double deg = atof(deg_str);
    double min = atof(nmea + deg_digits);
    double decimal = deg + (min / 60.0);

    if (hemi == 'S' || hemi == 'W')
        decimal *= -1;

    return decimal;
}

void display_latlon(double lat, double lon)
{
    char buf[17];
    LCD_Clear();
    LCD_Set_Cursor(1, 1);
    sprintf(buf, "Lat: %.4f", lat);
    LCD_String(buf);
    LCD_Set_Cursor(2, 1);
    sprintf(buf, "Lon: %.4f", lon);
    LCD_String(buf);
}
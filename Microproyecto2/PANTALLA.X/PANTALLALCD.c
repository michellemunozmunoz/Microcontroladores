/* 
 * File:   PANTALLALCD.c
 * Author: EQUIPO
 *
 * Created on 20 de octubre de 2025, 04:48 PM
 */

#include <xc.h>

// CONFIGURACIÓN DEL PIC18F4550
#pragma config FOSC = INTOSCIO_EC // Oscilador interno, RA6 como salida de reloj
#pragma config WDT = OFF          // Watchdog Timer desactivado
#pragma config LVP = OFF          // Low-Voltage Programming desactivado
#pragma config PBADEN = OFF       // PORTB digital al inicio
#pragma config MCLRE = ON         // Pin MCLR habilitado

#define _XTAL_FREQ 8000000  // Frecuencia de oscilador interno (8 MHz)

// Definición de pines del LCD
#define RS LATBbits.LATB0
#define EN LATBbits.LATB1
#define D4 LATBbits.LATB4
#define D5 LATBbits.LATB5
#define D6 LATBbits.LATB6
#define D7 LATBbits.LATB7

// --- Prototipos ---
void Lcd_Init(void);
void Lcd_Cmd(unsigned char cmd);
void Lcd_Char(unsigned char data);
void Lcd_String(const char *str);
void Lcd_Set_Cursor(unsigned char row, unsigned char column);
void Lcd_Clear(void);
void Lcd_Port(char nibble);

// --- Programa principal ---
void main(void) {
    // Configuración del oscilador interno a 8 MHz
    OSCCONbits.IRCF2 = 1;  // IRCF = 111 ? 8 MHz
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.SCS = 1;    // Selecciona el oscilador interno

    TRISB = 0x00;  // PORTB como salida
    LATB = 0x00;

    Lcd_Init();     // Inicializa LCD
    Lcd_Set_Cursor(1, 1);
    Lcd_String("Hello World"); // Muestra el texto

    while(1); // Bucle infinito
}

// --- Funciones del LCD ---
void Lcd_Port(char nibble) {
    D4 = (nibble >> 0) & 0x01;
    D5 = (nibble >> 1) & 0x01;
    D6 = (nibble >> 2) & 0x01;
    D7 = (nibble >> 3) & 0x01;
}

void Lcd_Cmd(unsigned char cmd) {
    RS = 0;
    Lcd_Port(cmd >> 4);  // Parte alta
    EN = 1; __delay_ms(2); EN = 0;
    Lcd_Port(cmd & 0x0F); // Parte baja
    EN = 1; __delay_ms(2); EN = 0;
}

void Lcd_Char(unsigned char data) {
    RS = 1;
    Lcd_Port(data >> 4);
    EN = 1; __delay_ms(2); EN = 0;
    Lcd_Port(data & 0x0F);
    EN = 1; __delay_ms(2); EN = 0;
}

void Lcd_Init(void) {
    __delay_ms(20);       // Espera inicial
    Lcd_Cmd(0x02);        // Inicialización 4 bits
    Lcd_Cmd(0x28);        // 4 bits, 2 líneas, 5x7
    Lcd_Cmd(0x0C);        // Display ON, cursor OFF
    Lcd_Cmd(0x06);        // Incrementa cursor
    Lcd_Cmd(0x01);        // Limpia pantalla
    __delay_ms(2);
}

void Lcd_Set_Cursor(unsigned char row, unsigned char column) {
    unsigned char pos;
    if(row == 1)
        pos = 0x80 + (column - 1);
    else
        pos = 0xC0 + (column - 1);
    Lcd_Cmd(pos);
}

void Lcd_String(const char *str) {
    while(*str)
        Lcd_Char(*str++);
}

void Lcd_Clear(void) {
    Lcd_Cmd(0x01);
    __delay_ms(2);
}


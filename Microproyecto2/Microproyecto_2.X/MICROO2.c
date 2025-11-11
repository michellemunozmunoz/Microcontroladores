/*
 * Proyecto FUSIONADO
 * PIC18F4550
 * GPS NEO-6M + RTC DS1307 (I2C en RC3/RC4) + DHT11 (RB0) + LCD 16x2
 *
 * Cableado asumido:
 *  LCD 4 bits:
 *      RS -> RB2
 *      EN -> RB3
 *      D4..D7 -> RD4..RD7   (usamos PORTD alto nibble)
 *
 *  GPS:
 *      RX del PIC -> TX del GPS (RC7)
 *      (solo recibimos)
 *
 *  RTC DS1307:
 *      SCL -> RC3
 *      SDA -> RC4
 *
 *  DHT11:
 *      DATA -> RB0
 *
 * Nota: todo está en este mismo archivo .c
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== CONFIGURACIÓN DEL PIC (tomada de tu configGPS.h) ========== */
#pragma config PLLDIV = 1
#pragma config CPUDIV = OSC1_PLL2
#pragma config USBDIV = 1
#pragma config FOSC = INTOSC_HS
#pragma config FCMEN = OFF
#pragma config IESO = OFF
#pragma config PWRT = OFF
#pragma config BOR = ON
#pragma config BORV = 3
#pragma config VREGEN = OFF
#pragma config WDT = OFF
#pragma config WDTPS = 32768
#pragma config CCP2MX = ON
#pragma config PBADEN = OFF
#pragma config LPT1OSC = OFF
#pragma config MCLRE = ON
#pragma config STVREN = ON
#pragma config LVP = OFF
#pragma config ICPRT = OFF
#pragma config XINST = OFF

#define _XTAL_FREQ 8000000UL

/* =========================================================
 * LCD 16x2 en 4 bits
 * RB2 = RS, RB3 = EN, RD4..RD7 = datos
 * ========================================================= */
#define RS         LATBbits.LATB2
#define EN         LATBbits.LATB3
#define RS_DIR     TRISBbits.TRISB2
#define EN_DIR     TRISBbits.TRISB3

#define D4         LATDbits.LATD4
#define D5         LATDbits.LATD5
#define D6         LATDbits.LATD6
#define D7         LATDbits.LATD7
#define D4_DIR     TRISDbits.TRISD4
#define D5_DIR     TRISDbits.TRISD5
#define D6_DIR     TRISDbits.TRISD6
#define D7_DIR     TRISDbits.TRISD7

static void LCD_Pulse(void){
    EN = 1;
    __delay_us(10);
    EN = 0;
    __delay_us(50);
}

static void LCD_Send4Bits(unsigned char data){
    D4 = (data >> 0) & 1;
    D5 = (data >> 1) & 1;
    D6 = (data >> 2) & 1;
    D7 = (data >> 3) & 1;
    LCD_Pulse();
}

void LCD_Command(unsigned char cmd){
    RS = 0;
    LCD_Send4Bits(cmd >> 4);
    LCD_Send4Bits(cmd & 0x0F);
    __delay_ms(2);
}

void LCD_Char(unsigned char data){
    RS = 1;
    LCD_Send4Bits(data >> 4);
    LCD_Send4Bits(data & 0x0F);
    __delay_us(50);
}

void LCD_String(const char *msg){
    while(*msg) LCD_Char(*msg++);
}

void LCD_String_xy(char row, char pos, const char *msg){
    char location = (row == 0) ? 0x80 : 0xC0;
    location += pos;
    LCD_Command(location);
    LCD_String(msg);
}

void LCD_Clear(void){
    LCD_Command(0x01);
    __delay_ms(2);
}

void LCD_Init(void){
    RS_DIR = 0;
    EN_DIR = 0;
    D4_DIR = 0;
    D5_DIR = 0;
    D6_DIR = 0;
    D7_DIR = 0;

    __delay_ms(50);

    RS = 0;
    EN = 0;

    LCD_Send4Bits(0x03);
    __delay_ms(5);
    LCD_Send4Bits(0x03);
    __delay_us(150);
    LCD_Send4Bits(0x03);
    LCD_Send4Bits(0x02);  // 4 bits

    LCD_Command(0x28);    // 4 bits, 2 lineas
    LCD_Command(0x0C);    // display on cursor off
    LCD_Command(0x06);    // incremento
    LCD_Command(0x01);    // clear
    __delay_ms(5);
}

/* =========================================================
 * I2C en RC3 (SCL) y RC4 (SDA) para DS1307
 * ========================================================= */
void I2C_Init(void){
    SSPCON1 = 0x28;   // I2C Master mode
    SSPCON2 = 0x00;
    SSPADD  = ((_XTAL_FREQ/4)/100000) - 1; // 100kHz
    SSPSTAT = 0x00;
    TRISCbits.TRISC3 = 1; // SCL
    TRISCbits.TRISC4 = 1; // SDA
}

void I2C_Start(void){
    SSPCON2bits.SEN = 1;
    while(SSPCON2bits.SEN);
}

void I2C_Stop(void){
    SSPCON2bits.PEN = 1;
    while(SSPCON2bits.PEN);
}

void I2C_Write(unsigned char data){
    SSPBUF = data;
    while(SSPSTATbits.BF);
    while((SSPCON2 & 0x1F) || SSPSTATbits.R_W);
}

unsigned char I2C_Read(unsigned char ack){
    unsigned char data;
    SSPCON2bits.RCEN = 1;
    while(!SSPSTATbits.BF);
    data = SSPBUF;
    SSPCON2bits.ACKDT = (ack)?0:1;
    SSPCON2bits.ACKEN = 1;
    while(SSPCON2bits.ACKEN);
    return data;
}

/* =========================================================
 * DS1307
 * ========================================================= */
#define DS1307_ADDRESS 0xD0

static unsigned char decToBcd(unsigned char v){ return (v/10)*16 + (v%10); }
static unsigned char bcdToDec(unsigned char v){ return (v/16)*10 + (v%16); }

void DS1307_Init(void){
    I2C_Init();
}

unsigned char DS1307_IsRunning(void){
    unsigned char sec;
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x00);
    I2C_Start();
    I2C_Write(DS1307_ADDRESS | 1);
    sec = I2C_Read(0);
    I2C_Stop();
    return !(sec & 0x80); // CH bit
}

void DS1307_SetTime(unsigned char h, unsigned char m, unsigned char s){
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x00);
    I2C_Write(decToBcd(s & 0x7F));
    I2C_Write(decToBcd(m));
    I2C_Write(decToBcd(h));
    I2C_Stop();
}

void DS1307_GetTime(unsigned char *h, unsigned char *m, unsigned char *s){
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x00);
    I2C_Start();
    I2C_Write(DS1307_ADDRESS | 1);
    *s = bcdToDec(I2C_Read(1));
    *m = bcdToDec(I2C_Read(1));
    *h = bcdToDec(I2C_Read(0));
    I2C_Stop();
}

void DS1307_SetDate(unsigned char d, unsigned char mt, unsigned char y){
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x04);
    I2C_Write(decToBcd(d));
    I2C_Write(decToBcd(mt));
    I2C_Write(decToBcd(y));
    I2C_Stop();
}

void DS1307_GetDate(unsigned char *d, unsigned char *mt, unsigned char *y){
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x04);
    I2C_Start();
    I2C_Write(DS1307_ADDRESS | 1);
    *d  = bcdToDec(I2C_Read(1));
    *mt = bcdToDec(I2C_Read(1));
    *y  = bcdToDec(I2C_Read(0));
    I2C_Stop();
}

/* =========================================================
 * UART SOLO RX para GPS
 * RC7 = RX
 * ========================================================= */
void UART_Init(long baudrate){
    TRISCbits.TRISC6 = 1;  // TX como entrada (no lo usamos)
    TRISCbits.TRISC7 = 1;  // RX como entrada

    SPBRG = (unsigned char)((_XTAL_FREQ / (64UL * baudrate)) - 1);
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 0;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 0;    // no transmitimos
    RCSTAbits.CREN = 1;
    PIR1bits.RCIF = 0;
}

unsigned char UART_Available(void){
    return PIR1bits.RCIF;
}

char UART_Read(void){
    if (RCSTAbits.OERR){
        RCSTAbits.CREN = 0;
        __delay_us(10);
        RCSTAbits.CREN = 1;
    }
    return RCREG;
}

/* =========================================================
 * GPS parsing (basado en tu GPS.c)
 * ========================================================= */
char nmea_buffer[256];
char lat_raw[16] = "", lon_raw[16] = "", alt_raw[16] = "", time_raw[16] = "";
char ns = 'N', ew = 'E';
float lat = 0.0, lon = 0.0, alt = 0.0;
unsigned long time_utc = 0;

float ConvertToDecimal(const char *value){
    if (value == NULL || strlen(value) < 4) return 0.0;

    float val = atof(value);
    int grados = (int)(val / 100);
    float minutos = val - (grados * 100.0);
    return grados + (minutos / 60.0);
}

unsigned char ParseGPGGA(char *sentence){
    if (strstr(sentence, "$GPGGA") == NULL) return 0;

    strcpy(lat_raw, "");
    strcpy(lon_raw, "");
    strcpy(alt_raw, "");
    strcpy(time_raw, "");
    ns = 'N';
    ew = 'E';

    char temp[256];
    strcpy(temp, sentence);

    char *token;
    int field = 0;

    token = strtok(temp, ",");
    while (token != NULL && field < 15){
        field++;
        switch(field){
            case 2: // tiempo UTC
                if (strlen(token) >= 6) strcpy(time_raw, token);
                break;
            case 3: // lat
                if (strlen(token) >= 4) strcpy(lat_raw, token);
                break;
            case 4: // N/S
                if (strlen(token) == 1) ns = token[0];
                break;
            case 5: // lon
                if (strlen(token) >= 4) strcpy(lon_raw, token);
                break;
            case 6: // E/W
                if (strlen(token) == 1) ew = token[0];
                break;
            case 10: // altitud
                if (strlen(token) > 0) strcpy(alt_raw, token);
                break;
        }
        token = strtok(NULL, ",");
    }

    if (strlen(lat_raw) < 4 || strlen(lon_raw) < 4) return 0;

    lat      = ConvertToDecimal(lat_raw);
    lon      = ConvertToDecimal(lon_raw);
    alt      = atof(alt_raw);
    time_utc = atol(time_raw);

    if (ns == 'S') lat = -lat;
    if (ew == 'W') lon = -lon;

    return 1;
}

/* =========================================================
 * DHT11 en RB0 (tal cual tu código)
 * ========================================================= */
#define DHT11_PIN   PORTBbits.RB0
#define DHT11_TRIS  TRISBbits.TRISB0

uint8_t humidity_int=0, humidity_dec=0, temp_int=0, temp_dec=0, checksum=0;
unsigned char dht_ok = 0;

static uint8_t waitForState(uint8_t state, uint16_t timeout_us){
    while(((DHT11_PIN & 1) != state)){
        if (timeout_us-- == 0) return 0;
        __delay_us(1);
    }
    return 1;
}

void DHT11_Start(void){
    DHT11_TRIS = 0;        // salida
    LATBbits.LATB0 = 0;    // bajo
    __delay_ms(18);        // pulso de inicio
    LATBbits.LATB0 = 1;    // soltar
    __delay_us(20);
    DHT11_TRIS = 1;        // entrada
}

uint8_t DHT11_CheckResponse(void){
    if (!waitForState(0, 200)) return 0;
    if (!waitForState(1, 200)) return 0;
    return 1;
}

uint8_t DHT11_ReadByte(void){
    uint8_t i, data=0;
    for(i=0;i<8;i++){
        if (!waitForState(0, 200)) return 0xFF;
        if (!waitForState(1, 400)) return 0xFF;
        __delay_us(40);
        data <<= 1;
        if (DHT11_PIN) data |= 1;
    }
    return data;
}

/* =========================================================
 * Variables globales extra (RTC + display)
 * ========================================================= */
unsigned char rtc_h=0, rtc_m=0, rtc_s=0, rtc_d=0, rtc_mt=0, rtc_y=0;

/* =========================================================
 * Pantallas:
 * 0 -> GPS Lat/Lon
 * 1 -> GPS Alt/UTC
 * 2 -> RTC Fecha/Hora
 * 3 -> DHT11 Temp/Hum
 * ========================================================= */
void DisplaySystemData(void){
    static unsigned char screen = 0;
    char buffer[17];

    switch(screen){
        case 0:
            LCD_Clear();
            sprintf(buffer, "Lat:%9.5f", lat);
            LCD_String_xy(0,0,buffer);
            sprintf(buffer, "Lon:%9.5f", lon);
            LCD_String_xy(1,0,buffer);
            break;
        case 1:
            LCD_Clear();
            sprintf(buffer, "Alt:%6.1f m", alt);
            LCD_String_xy(0,0,buffer);
            if (time_utc){
                unsigned int hh = time_utc/10000;
                unsigned int mm = (time_utc%10000)/100;
                unsigned int ss = time_utc%100;
                sprintf(buffer, "UTC %02u:%02u:%02u", hh,mm,ss);
            } else {
                sprintf(buffer, "UTC --:--:--");
            }
            LCD_String_xy(1,0,buffer);
            break;
        case 2:
            LCD_Clear();
            sprintf(buffer, "Fecha:%02u/%02u/20%02u", rtc_d, rtc_mt, rtc_y);
            LCD_String_xy(0,0,buffer);
            sprintf(buffer, "Hora :%02u:%02u:%02u", rtc_h, rtc_m, rtc_s);
            LCD_String_xy(1,0,buffer);
            break;
        default:
            LCD_Clear();
            if (dht_ok){
                sprintf(buffer, "RH: %d.%d %%", humidity_int, humidity_dec);
                LCD_String_xy(0,0,buffer);
                sprintf(buffer, "T: %d.%d", temp_int, temp_dec);
                LCD_String_xy(1,0,buffer);
                LCD_Char(0xDF); // °
                LCD_Char('C');
            } else {
                LCD_String_xy(0,0,"DHT11 sin dato");
            }
            break;
    }

    screen = (screen + 1) % 4;
}

/* =========================================================
 * MAIN
 * ========================================================= */
void main(void){
    // Reloj interno 8MHz
    OSCCONbits.IRCF = 0b111;
    // Pines digitales
    ADCON1 = 0x0F;
    CMCON  = 0x07;

    LCD_Init();
    UART_Init(9600);
    DS1307_Init();

    LCD_Clear();
    LCD_String_xy(0,0,"GPS+RTC+DHT11");
    LCD_String_xy(1,0,"Iniciando...");
    __delay_ms(1500);

    // Si el RTC no corre, ponemos una hora/fecha de ejemplo
    if (!DS1307_IsRunning()){
        DS1307_SetTime(15, 32, 0);   // 15:32:00
        DS1307_SetDate(5, 11, 25);   // 05/11/2025
    }

    // Variables de recepción GPS
    unsigned int index = 0;
    char received_char;
    unsigned char sentence_started = 0;
    unsigned char valid_data = 0;

    // "timers" por software (ms)
    unsigned long timeout_gps   = 0;
    unsigned long display_timer = 0;
    unsigned long rtc_timer     = 0;
    unsigned long dht_timer     = 0;

    while(1){
        if (UART_Available()){
            received_char = UART_Read();

            if (received_char == '$'){
                index = 0;
                sentence_started = 1;
                nmea_buffer[index++] = received_char;
            } else if (sentence_started){
                if (received_char == '\n'){
                    nmea_buffer[index] = '\0';
                    if (strstr(nmea_buffer, "$GPGGA")){
                        valid_data = ParseGPGGA(nmea_buffer);
                        if (valid_data){
                            DisplaySystemData();
                            display_timer = 0;
                        }
                    }
                    sentence_started = 0;
                    index = 0;
                    timeout_gps = 0;
                } else if (index < sizeof(nmea_buffer)-1){
                    nmea_buffer[index++] = received_char;
                }
            }
        } else {
            __delay_ms(1);
            timeout_gps++;
            display_timer++;
            rtc_timer++;
            dht_timer++;

            // cada 1s -> leer RTC
            if (rtc_timer > 1000){
                DS1307_GetTime(&rtc_h, &rtc_m, &rtc_s);
                DS1307_GetDate(&rtc_d, &rtc_mt, &rtc_y);
                rtc_timer = 0;
            }

            // cada 1.1s -> leer DHT11
            if (dht_timer > 1100){
                DHT11_Start();
                if (DHT11_CheckResponse()){
                    uint8_t h1 = DHT11_ReadByte();
                    uint8_t h2 = DHT11_ReadByte();
                    uint8_t t1 = DHT11_ReadByte();
                    uint8_t t2 = DHT11_ReadByte();
                    uint8_t cs = DHT11_ReadByte();
                    if ((uint16_t)h1 + h2 + t1 + t2 == cs){
                        humidity_int = h1;
                        humidity_dec = h2;
                        temp_int     = t1;
                        temp_dec     = t2;
                        checksum     = cs;
                        dht_ok       = 1;
                    } else {
                        dht_ok = 0;
                    }
                } else {
                    dht_ok = 0;
                }
                dht_timer = 0;
            }

            // si en 10s no hay GPS -> aviso
            if (timeout_gps > 10000 && !valid_data){
                LCD_Clear();
                LCD_String_xy(0,0,"SIN SENAL GPS");
                LCD_String_xy(1,0,"RTC+DHT11 OK");
                timeout_gps = 0;
            }

            // cada 5s -> cambiar pantalla
            if (display_timer > 5000){
                DisplaySystemData();
                display_timer = 0;
            }
        }
    }
}

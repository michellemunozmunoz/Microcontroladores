#include "config.h"
#include "lcd.h"
#include "ds1307.h"
#include "gps.h"
#include "dht11.h"
#include <stdio.h>   // sprintf

static void Mostrar_RTC(void) {
    unsigned char hh, mm, ss;
    unsigned char dd, MM, yy;
    char linea[17];

    DS1307_GetTime(&hh, &mm, &ss);
    DS1307_GetDate(&dd, &MM, &yy);

    LCD_Clear();
    sprintf(linea, "H:%02u:%02u:%02u", hh, mm, ss);
    LCD_String_xy(0, 0, linea);
    sprintf(linea, "F:%02u/%02u/20%02u", dd, MM, yy);
    LCD_String_xy(1, 0, linea);
}

static void Mostrar_GPS(void) {
    char linea[17];
    LCD_Clear();
    if (GPS_HasFix()) {
        float lat = GPS_GetLat();
        unsigned long utc = GPS_GetUTC();

        sprintf(linea, "Lat:%2.4f", lat);
        LCD_String_xy(0, 0, linea);

        if (utc > 0) {
            unsigned int h = (unsigned int)(utc / 10000UL);
            unsigned int m = (unsigned int)((utc % 10000UL) / 100UL);
            unsigned int s = (unsigned int)(utc % 100UL);
            sprintf(linea, "UTC %02u:%02u:%02u", h, m, s);
        } else {
            sprintf(linea, "GPS sin hora");
        }
        LCD_String_xy(1, 0, linea);
    } else {
        LCD_String_xy(0, 0, "GPS sin datos");
        LCD_String_xy(1, 0, "Esperando...");
    }
}

static void Mostrar_DHT11(void) {
    char linea[17];

    LCD_Clear();
    if (DHT11_ReadFrame()) {
        // Línea 1: humedad
        sprintf(linea, "RH:%d.%d %%", dht_hum_int, dht_hum_dec);
        LCD_String_xy(0, 0, linea);
        // Línea 2: temperatura
        sprintf(linea, "T:%d.%d ", dht_temp_int, dht_temp_dec);
        LCD_String_xy(1, 0, linea);
        LCD_Char(0xDF); // °
        LCD_Char('C');
    } else {
        LCD_String_xy(0, 0, "DHT11 sin resp");
        LCD_String_xy(1, 0, "reintente...");
    }
}

void main(void) {
    // oscilador ya en config.h
    ADCON1 = 0x0F;
    CMCON  = 0x07;

    LCD_Init();
    DS1307_Init();
    GPS_UART_Init(9600);

    // RA0 como entrada para DHT11 (lo cambia DHT11_Start pero lo dejamos aquí)
    TRISAbits.TRISA0 = 1;

    LCD_Clear();
    LCD_String_xy(0, 0, "RTC+GPS+DHT11");
    __delay_ms(1200);

    while (1) {
        // 1) RTC
        Mostrar_RTC();
        for (unsigned int i = 0; i < 3000; i++) {
            GPS_Task();     // seguir recibiendo NMEA
            __delay_ms(1);
        }

        // 2) GPS
        Mostrar_GPS();
        for (unsigned int i = 0; i < 3000; i++) {
            GPS_Task();
            __delay_ms(1);
        }

        // 3) DHT11
        Mostrar_DHT11();
        for (unsigned int i = 0; i < 3000; i++) {
            GPS_Task();   // de paso seguimos alimentando el parser
            __delay_ms(1);
        }
    }
}


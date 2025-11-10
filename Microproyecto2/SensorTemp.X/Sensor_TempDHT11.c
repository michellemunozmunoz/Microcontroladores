/* 
 * Author: EQUIPO
 *
 * Created on 6 de octubre de 2025, 11:30 AM
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <pic18f4550.h>
#include "Config.h"
#include "LCD.h"

#define DHT11_PIN PORTBbits.RB0
#define DHT11_TRIS TRISBbits.TRISB0

uint8_t humidity_int, humidity_dec, temp_int, temp_dec, checksum;

static uint8_t waitForState(uint8_t state, uint16_t timeout_us)
{
    while(((DHT11_PIN & 1) != state)) {
        if(timeout_us-- == 0) return 0;
        __delay_us(1);
    }
    return 1;
}

void DHT11_Start(void)
{
    DHT11_TRIS = 0;
    LATBbits.LATB0 = 0;
    __delay_ms(18);
    LATBbits.LATB0 = 1;
    __delay_us(20);
    DHT11_TRIS = 1;
}

uint8_t DHT11_CheckResponse(void)
{
    if(!waitForState(0, 100)) return 0;
    if(!waitForState(1, 100)) return 0;
    return 1;
}

uint8_t DHT11_ReadData(void)
{
    uint8_t i, data = 0;
    for(i = 0; i < 8; i++)
    {
        if(!waitForState(0, 1000)) return 0xFF;
        if(!waitForState(1, 1000)) return 0xFF;
        __delay_us(40);
        if(DHT11_PIN & 1) data = (data << 1) | 1;
        else data = (data << 1);
    }
    return data;
}

void main(void) {
    char value[16];
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 0b10;
    ADCON1 = 0x0F;
    TRISBbits.TRISB0 = 1;

    LCD_Init();
    LCD_String_xy(0, 0, "BIENVENIDO");
    LCD_String_xy(1, 0, "MICROCONTROLADOR");
    __delay_ms(1000);
    LCD_Clear();

    while (1) {
        DHT11_Start();
        if (DHT11_CheckResponse()) {
            uint8_t h1 = DHT11_ReadData();
            if(h1 == 0xFF) { __delay_ms(1000); /*LCD_Clear(); LCD_String_xy(0,0,"TIMEOUT READ"); LCD_String_xy(1,0,"ERROR"); __delay_ms(2000); */continue; }
            uint8_t h2 = DHT11_ReadData();
            if(h2 == 0xFF) { __delay_ms(1000); /* LCD_Clear(); LCD_String_xy(0,0,"TIMEOUT READ"); LCD_String_xy(1,0,"ERROR"); __delay_ms(2000); */ continue; }
            uint8_t t1 = DHT11_ReadData();
            if(t1 == 0xFF) { __delay_ms(1000); /* LCD_Clear(); LCD_String_xy(0,0,"TIMEOUT READ"); LCD_String_xy(1,0,"ERROR"); __delay_ms(2000); */ continue; }
            uint8_t t2 = DHT11_ReadData();
            if(t2 == 0xFF) { __delay_ms(1000);/* LCD_Clear(); LCD_String_xy(0,0,"TIMEOUT READ"); LCD_String_xy(1,0,"ERROR"); __delay_ms(2000); */ continue; }
            uint8_t cs = DHT11_ReadData();
            if(cs == 0xFF) { __delay_ms(1000); /* LCD_Clear(); LCD_String_xy(0,0,"TIMEOUT READ"); LCD_String_xy(1,0,"ERROR"); __delay_ms(2000); */ continue; }

            humidity_int = h1;
            humidity_dec = h2;
            temp_int = t1;
            temp_dec = t2;
            checksum = cs;

            if (((uint16_t)humidity_int + humidity_dec + temp_int + temp_dec) == checksum) {
                LCD_Clear();
                LCD_String_xy(0, 0, "RH: ");
                sprintf(value, "%d.%d %%", humidity_int, humidity_dec);
                LCD_String(value);
                LCD_String_xy(1, 0, "TEMP: ");
                sprintf(value, "%d.%d ", temp_int, temp_dec);
                LCD_String(value);
                LCD_Char(0xdf);
                LCD_Char('C');
            } else {
                __delay_ms(1000);
                LCD_Clear();
                //LCD_String_xy(0,0,"CHECKSUM!!");
                //LCD_String_xy(1,0,"ERROR!!");
            }
        } else {
            __delay_ms(1000);
            LCD_Clear();
            //LCD_String_xy(0,0,"UNRESPONSIVE");
            //LCD_String_xy(1,0,"ERROR!!");
        }
        __delay_ms(2000);
    }
}



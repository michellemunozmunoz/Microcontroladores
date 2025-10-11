/* 
 * File:   SENSOR_IC2.c
 * Author: EQUIPO
 *
 * Created on 10 de octubre de 2025, 03:36 PM
 */

#include <xc.h>
#include <stdint.h>
#include <studio.h>
#include <string.h>

#pragma config FOSC = INTOSCIO_EC   //Oscilador interno, RA6/RA7 I/O
#pragma config PLLDIV = 1            
#pragma config CPUDIV = OSC1_PLL2         
#pragma config USBDIV = 2
#pragma config WDTEN = OFF   
#pragma config LVP = OFF            
#pragma config BOR = OFF            
#pragma config MCLRE = ON


#define _XTAL_FREQ 8000000

// RS RA0
// EN RA1
// D4 RD0
// D5 RD1
// D6 RD2
// D7 RD3

#define LCD_RS_LAT LATAbits.LATA0
#define LCD_RS_TRIS TRISAbits.TRISA0 
#define LCD_EN_LAT LATAbits.LATA1 
#define LCD_EN_TRIS LATAbits.TRISA1


#define LCD_D_TRIS TRISDbits.TRISD
#define LCD_D_LAT LATD 

//I2C por software en RB0 (SDA) y RB1 (SCL)

#define SDA_TRIS TRISBbits.TRISB0 
#define SDA_PORT PORTBbits.RB0  
#define SDA_LAT LATBbits.LATB0

#define SCL_TRIS TRISBbits.TRISB1 
#define SCL_PORT PORTBbits.RB1  
#define SCL_LAT LATBbits.LATB1

//Ajuste de tiempos I2C 

static inline void 12c_delay_half(void){__delay_us(5); }  //5us
static inline void 12c_delay(void){__delay_us(10); }  //10us

void i2c_init(void) {
    LATBbits.LATB0 = 1; 
    LATBbits.LATB1 = 1;
    SDA_TRIS = 1; //SDA release 
    SCL_TRIS = 1; //SCL release    
}

static inline void sda_release(void){LATBbits.LATB0 = 1; SDA_TRIS =1; }
static inline void sda_low(void){LATBbits.LATB0 = 0; SDA_TRIS =0; }
static inline uint8_t sda_read(void) {return PORTBbits.RB0; }

static inline void scl_release(void){LATBbits.LATB1 = 1; SCL_TRIS =1; }
static inline void scl_low(void){LATBbits.LATB1 = 0; SCL_TRIS =0; }
static inline uint8_t sca_read(void) {return PORTBbits.RB1; } 






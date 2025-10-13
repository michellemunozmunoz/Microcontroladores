/* 
 * File:   SENSOR_IC2.c
 * Author: EQUIPO
 *
 * Created on 10 de octubre de 2025, 03:36 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//Configuración básica
#pragma config FOSC = INTOSCIO_EC   //Oscilador interno, RA6/RA7 E/S         
#pragma config WDT = OFF   
#pragma config LVP = OFF                       
#pragma config MCLRE = ON
#pragma config PBADEN = OFF 

#define _XTAL_FREQ 8000000    //Frecuencia de reloj interno

//I2C SOFTWARE
#define SDA_TRIS TRISBbits.TRISB0
#define SCL_TRIS TRISBbits.TRISB1 
#define SDA_LAT LATBbits.LATB0 
#define SCL_LAT LATBbits.LATB1
#define SDA_PORT PORTBbits.RB0

void I2C_Delay(void)
{__delay_us(10);}

void I2C_Init(void){
    SDA_TRIS = 1; SCL_TRIS = 1;
    SDA_LAT = 1; SCL_LAT = 1; 
}

void I2C_Start(void) { 
    SDA_TRIS = 0; SDA_LAT = 1;
    SCL_TRIS = 0; SCL_LAT = 1; 
    I2C_Delay();
    SDA_LAT= 0; I2C_Delay();
    SCL_LAT = 0; 
}

void I2C_Stop(void) { 
    SDA_TRIS = 0; SDA_LAT = 0; 
    I2C_Delay();
    SCL_LAT= 1; I2C_Delay();
    SDA_LAT = 1; I2C_Delay();
}

void I2C_WriteBit(uint8_t bit) {
    SDA_TRIS = 0; SDA_LAT = bit; 
    I2C_Delay(); SCL_LAT= 1; 
    I2C_Delay(); SCL_LAT = 0;  
}

uint8_t I2C_ReadBit(void) {
    uint8_t bit;
    SDA_TRIS = 1;
    I2C_Delay(); SCL_LAT= 1; 
    I2C_Delay(); bit = SDA_PORT;
    SCL_LAT=0;
    return bit; 
}

uint8_t I2C_Write(uint8_t data) {
    for (int i = 0; i<8; i++) {
        I2C_WriteBit((data & 0x80)!=0);
        data <<= 1;
        }
        return !I2C_ReadBit();    //ACK = 0    
        }

uint8_t I2C_Read(uint8_t ack) {
    uint8_t data = 0;
    SDA_TRIS = 1;
    for (int i = 0; i<8; i++) {
        data <<= 1;
        if(I2C_ReadBit())data|=1;   
        }
        I2C_WriteBit(!ack);
        return data;
    }
    
    //LCD 16x2 MODO 4 BITS
    
#define LCD_RS LATAbits.LATA0
#define LCD_EN LATAbits.LATA1
#define LCD_D4 LATAbits.LATA2
#define LCD_D5 LATAbits.LATA3
#define LCD_D6 LATAbits.LATA4
#define LCD_D7 LATAbits.LATA5
#define LCD_PORT_DIR TRISA
    
void LCD_Pulse(void){
    LCD_EN = 1;__delay_us(40);
    LCD_EN = 0;__delay_us(40);
}

void LCD_Cmd(uint8_t cmd){
    LCD_RS = 0;
    LCD_D4 = (cmd>>4)&1;
    LCD_D5 = (cmd>>5)&1;
    LCD_D6 = (cmd>>6)&1;
    LCD_D7 = (cmd>>7)&1;
    LCD_Pulse();
    
    LCD_D4 = cmd&1;
    LCD_D5 = (cmd>>1)&1;
    LCD_D6 = (cmd>>2)&1;
    LCD_D7 = (cmd>>3)&1;
    LCD_Pulse();
    
    __delay_ms(2);  
}

void LCD_Init(void){
    LCD_PORT_DIR=0x00;
    __delay_ms(20);
    LCD_Cmd(0x02);
    LCD_Cmd(0x28);
    LCD_Cmd(0x0C);
    LCD_Cmd(0x06);
    LCD_Cmd(0x01);
     __delay_ms(2);  
}

void LCD_SetCursor(uint8_t row, uint8_t col){
    uint8_t address = (row == 1)? 0x80 : 0xC0;
    address +=(col-1);
    LCD_Cmd(address);   
}

void LCD_WriteChar(char c){
    LCD_RS = 1;
    LCD_D4 = (c>>4)&1;
    LCD_D5 = (c>>5)&1;
    LCD_D6 = (c>>6)&1;
    LCD_D7 = (c>>7)&1;
    LCD_Pulse();
    LCD_D4 = c & 1;
    LCD_D5 = (c>>1)&1;
    LCD_D6 = (c>>2)&1;
    LCD_D7 = (c>>3)&1;
    LCD_Pulse();
    __delay_us(100);         
}

void LCD_WriteString(const char*str){
    while(*str)LCD_WriteChar(*str++);
}






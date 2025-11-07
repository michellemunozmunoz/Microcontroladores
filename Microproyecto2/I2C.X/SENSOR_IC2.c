/* 
 * File:   SENSOR_IC2.c
 * Author: EQUIPO
 *
 * Created on 10 de octubre de 2025, 03:36 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h>

// === CONFIGURACIÓN ===
#pragma config FOSC = INTOSCIO_EC
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF
#pragma config MCLRE = ON

#define _XTAL_FREQ 8000000

// === LCD ===
#define LCD_RS LATBbits.LATB2
#define LCD_EN LATBbits.LATB3
#define LCD_D4 LATDbits.LATD4
#define LCD_D5 LATDbits.LATD5
#define LCD_D6 LATDbits.LATD6
#define LCD_D7 LATDbits.LATD7

// === I2C software ===
#define SDA LATBbits.LATB0
#define SCL LATBbits.LATB1
#define SDA_TRIS TRISBbits.TRISB0
#define SCL_TRIS TRISBbits.TRISB1
#define SDA_READ PORTBbits.RB0

// === Dirección TMD27713 ===
#define TMD27713_ADDR  0x39

void I2C_Delay(void){ __delay_us(5); }

void I2C_Init(void){ SDA_TRIS=1; SCL_TRIS=1; }

void I2C_Start(void){
    SDA_TRIS=0; SDA=1; SCL_TRIS=0; SCL=1; I2C_Delay();
    SDA=0; I2C_Delay(); SCL=0;
}

void I2C_Stop(void){
    SDA_TRIS=0; SDA=0; SCL=0; I2C_Delay();
    SCL=1; SDA=1; I2C_Delay();
}

uint8_t I2C_Write(uint8_t data){
    for(uint8_t i=0;i<8;i++){
        SDA_TRIS=0; SDA=(data & 0x80)?1:0;
        I2C_Delay(); SCL=1; I2C_Delay(); SCL=0;
        data <<= 1;
    }
    SDA_TRIS=1;
    SCL=1; I2C_Delay();
    uint8_t ack = !SDA_READ;
    SCL=0;
    return ack;
}

uint8_t I2C_Read(uint8_t ack){
    uint8_t data=0;
    SDA_TRIS=1;
    for(uint8_t i=0;i<8;i++){
        SCL=1; data <<= 1; if(SDA_READ) data |= 1;
        I2C_Delay(); SCL=0;
    }
    SDA_TRIS=0; SDA=!ack;
    I2C_Delay(); SCL=1; I2C_Delay(); SCL=0; SDA_TRIS=1;
    return data;
}

// === LCD ===
void LCD_Pulse(void){ LCD_EN=1; __delay_us(40); LCD_EN=0; __delay_us(40); }

void LCD_Nibble(uint8_t nib){
    LCD_D4=(nib>>0)&1; LCD_D5=(nib>>1)&1;
    LCD_D6=(nib>>2)&1; LCD_D7=(nib>>3)&1;
    LCD_Pulse();
}

void LCD_Cmd(uint8_t cmd){
    LCD_RS=0; LCD_Nibble(cmd>>4); LCD_Nibble(cmd&0x0F); __delay_ms(2);
}

void LCD_Char(char data){
    LCD_RS=1; LCD_Nibble(data>>4); LCD_Nibble(data&0x0F); __delay_us(50);
}

void LCD_String(const char *txt){ while(*txt) LCD_Char(*txt++); }

void LCD_SetCursor(uint8_t row,uint8_t col){
    uint8_t addr=(row==1)?0x80:0xC0; addr += (col-1); LCD_Cmd(addr);
}

void LCD_Init(void){
    TRISBbits.TRISB2=0; TRISBbits.TRISB3=0; TRISD &= 0x0F;
    __delay_ms(20);
    LCD_RS=0; LCD_Nibble(0x03); __delay_ms(5);
    LCD_Nibble(0x03); __delay_us(200);
    LCD_Nibble(0x03); LCD_Nibble(0x02);
    LCD_Cmd(0x28); LCD_Cmd(0x0C); LCD_Cmd(0x06); LCD_Cmd(0x01);
    __delay_ms(2);
}

// === TMD27713 ===
void TMD27713_WriteReg(uint8_t reg, uint8_t val){
    I2C_Start();
    I2C_Write((TMD27713_ADDR<<1)|0);
    I2C_Write(0x80|reg);
    I2C_Write(val);
    I2C_Stop();
}

uint8_t TMD27713_ReadReg(uint8_t reg){
    uint8_t val;
    I2C_Start();
    I2C_Write((TMD27713_ADDR<<1)|0);
    I2C_Write(0x80|reg);
    I2C_Start();
    I2C_Write((TMD27713_ADDR<<1)|1);
    val=I2C_Read(0);
    I2C_Stop();
    return val;
}

void TMD27713_Init(void){
    __delay_ms(50);

    // Apagar primero
    TMD27713_WriteReg(0x00, 0x00);
    __delay_ms(10);

    // ATIME - tiempo integración ALS
    TMD27713_WriteReg(0x01, 0xD6); // 100 ms
    // PTIME - tiempo integración proximidad
    TMD27713_WriteReg(0x02, 0xFF); // 2.7 ms
    // WTIME - tiempo espera
    TMD27713_WriteReg(0x03, 0xEE);
    // Control de corriente LED e IR
    TMD27713_WriteReg(0x0E, 0x20); // LED 100mA, PDRIVE bits=10
    // Control de ganancia
    TMD27713_WriteReg(0x0F, 0x20); // AGAIN=1x, PGAIN=2x

    // Activar alimentación y sensores
    TMD27713_WriteReg(0x00, 0x0F); // PON | AEN | PEN | WEN
    __delay_ms(20);
}

uint16_t TMD27713_ReadProximity(void){
    uint8_t low = TMD27713_ReadReg(0x9C);
    uint8_t high = TMD27713_ReadReg(0x9D);
    return ((uint16_t)high << 8) | low;
}

// === MAIN ===
void main(void){
    OSCCON=0b01110000; ADCON1=0x0F; CMCON=0x07;

    LCD_Init();
    I2C_Init();

    LCD_SetCursor(1,1); LCD_String("KS0269 Inicializa");
    TMD27713_Init();
    LCD_Cmd(0x01);
    LCD_SetCursor(1,1); LCD_String("Keyestudio KS0269");

    while(1){
        uint16_t prox = TMD27713_ReadProximity();
        char buf[16];
        sprintf(buf,"Prox:%5u", prox);
        LCD_SetCursor(2,1);
        LCD_String(buf);
        __delay_ms(200);
    }
}
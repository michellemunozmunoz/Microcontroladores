/* 
 * File:   LCD.c
 * Author: EQUIPO
 *
 * Created on 10 de noviembre de 2025, 11:51 AM
 */

#include "LCD.h"

void LCD_Init(void)
{
    LCD_Port = 0;
    RS_TRIS = 0;
    EN_TRIS = 0;
    RS = 0;
    EN = 0;

    __delay_ms(15);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(CMD_CLEAR_LCD);
    LCD_Command(0x0C);
    LCD_Command(0x06);
}

void LCD_Command(unsigned char cmd)
{
    ldata = (ldata & 0x0F) | (unsigned char)(0xF0 & cmd);
    RS = 0;
    EN = 1;
    NOP();
    EN = 0;
    __delay_ms(1);

    ldata = (ldata & 0x0F) | (unsigned char)((cmd << 4) & 0xF0);
    EN = 1;
    NOP();
    EN = 0;
    __delay_ms(3);
}

void LCD_Char(unsigned char dat)
{
    ldata = (ldata & 0x0F) | (unsigned char)(0xF0 & dat);
    RS = 1;
    EN = 1;
    NOP();
    EN = 0;
    __delay_ms(1);

    ldata = (ldata & 0x0F) | (unsigned char)((dat << 4) & 0xF0);
    EN = 1;
    NOP();
    EN = 0;
    __delay_ms(3);
}

void LCD_String(const char *msg)
{
    while((*msg) != 0)
    {		
        LCD_Char(*msg);
        msg++;	
    }	
}

void LCD_String_xy(char row,char pos,const char *msg)
{
    unsigned char location = 0;
    if(row < 1)
    {
        location = (unsigned char)((0x80) | ((pos) & 0x0F));
        LCD_Command(location);
    }
    else
    {
        location = (unsigned char)((0xC0) | ((pos) & 0x0F));
        LCD_Command(location);    
    }  
    LCD_String(msg);
}

void LCD_Clear(void)
{
    LCD_Command(CMD_CLEAR_LCD);
}


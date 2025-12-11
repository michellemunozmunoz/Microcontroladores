/* 
 * Author: EQUIPO
 *
 * Created on 6 de octubre de 2025, 11:30 AM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <pic18f4550.h>
#include "Config.h"

#define RS LATBbits.LATB2
#define EN LATBbits.LATB3
#define RS_TRIS TRISBbits.TRISB2
#define EN_TRIS TRISBbits.TRISB3
#define ldata LATD
#define LCD_Port TRISD
#define CMD_CLEAR_LCD 0x01

void LCD_Init(void);
void LCD_Command(unsigned char);
void LCD_Char(unsigned char x);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void LCD_Clear(void);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */




/* 
 * File:   LEDPARPADEO.c
 * Author: EQUIPO
 *
 * Created on 3 de octubre de 2025, 07:56 AM
 */

#include <xc.h>

#pragma config FOSC = INTOSCIO_EC   // Oscilador interno (RA6 y RA7 como I/O)
#pragma config WDT = OFF            // Watchdog desactivado
#pragma config LVP = OFF            // Desactiva programación por bajo voltaje (libera RB5)
#pragma config PBADEN = OFF         // PORTB como digital al reset


#define _XTAL_FREQ 8000000        // Frecuencia de 8 MHz

void main(void){
    OSCCON=0x72;                  //Configuración del reloj (8 MHz)
    TRISBbits.TRISB0=0;           //RB0 como salida
    LATBbits.LATB0=0;             //LED apagado al inicio 
    
    while(1){
        TRISBbits.TRISB0=1;       //LED ON
        __delay_ms(1000);         //1 segundo encendido
        
        LATBbits.LATB0=0;         //LED OFF
        __delay_ms(2000);         //2 segundos apagado
        
    } 

}


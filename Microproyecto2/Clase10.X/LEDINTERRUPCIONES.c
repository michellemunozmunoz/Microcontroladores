/* 
 * File:   LEED.c
 * Author: EQUIPO
 *
 * Created on 3 de octubre de 2025, 12:23 PM
 */
#include <xc.h>

#pragma config FOSC = INTOSCIO_EC   // Oscilador interno (RA6 y RA7 como I/O)
#pragma config WDT = OFF            // Watchdog desactivado
#pragma config LVP = OFF            // Desactiva programación por bajo voltaje
#pragma config PBADEN = OFF         // PORTB como digital al reset
#pragma config MCLRE = OFF          // Deshabilita MCLR, RE3 como entrada digital

#define _XTAL_FREQ 8000000        // Frecuencia de 8 MHz

void __interrupt() ISR(void) {
    if (INTCONbits.INT0IF) {          
        LATDbits.LATD0 = !LATDbits.LATD0;   // Cambiar estado del LED
        INTCONbits.INT0IF = 0;              // Limpiar
    }
}

void main(void) {
    
    OSCCONbits.IRCF = 0b111;   // IRCF2:0 = 111 ? 8 MHz
    OSCCONbits.SCS = 0b10;     // Usa oscilador interno

   
    TRISBbits.TRISB0 = 1;      // RB0 como entrada (botón)
    TRISDbits.TRISD0 = 0;      // RD0 como salida (LED)
    LATDbits.LATD0 = 0;        // LED apagado al inicio
    ADCON1 = 0x0F;             // Todos los pines como digitales

   
    INTCON2bits.INTEDG0 = 0;   // INT0 en flanco de bajada
    INTCONbits.INT0IF = 0;     // Limpiar INT0
    INTCONbits.INT0IE = 1;     // Habilitar interrupción externa INT0
    INTCONbits.GIE = 1;        // Habilitar interrupciones globales

    while (1) {
       
    }
}


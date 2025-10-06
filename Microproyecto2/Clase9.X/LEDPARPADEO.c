/* 
 * File:   LEDPARPADEO.c
 * Author: EQUIPO
 *
 * Created on 3 de octubre de 2025, 07:56 AM
 */

//Punto B

#include <xc.h>
//#include <pic18f4550.h>

#pragma config FOSC = INTOSCIO_EC   // Oscilador interno (RA6 y RA7 como I/O)
#pragma config WDT = OFF            // Watchdog desactivado
#pragma config LVP = OFF            // Desactiva programación por bajo voltaje (libera RB5)
#pragma config PBADEN = OFF         // PORTB como digital al reset


#define _XTAL_FREQ 8000000        // Frecuencia de 8 MHz 

void main(void){
    OSCCON=0x72;                   //Configuración del oscilador de 8MHz
    TRISBbits.TRISB0=0     //RB0 como salida digital
    LATBbits.LATB0=0       //LED apagado al inicio 
    unsigned int tiempo_total=20000; //20 segundos
    unsigned int tiempo_ciclo=5*(500+500); 
    
    while (tiempo_total>0){
        for (int i=0;i<5;i++){
            LATBbits.LATB0=1  //LED ON
            _delay_ms(500);
            LATBbits.LATB0=0  //LED OFF
            _delay_ms(500);
        }
        tiempo_total-=tiempo_ciclo; //5s
    }
    
    for (int i=0;i<2;i++){
        LATBbits.LATB0=1 //LED ON
        _delay_ms(1000);
        LATBbits.LATB0=0 //LED OFF
        _delay_ms(1000);
    }
    
    LATBbits.LATB0=0
    while(1);
}


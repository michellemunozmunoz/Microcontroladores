/* 
 * File:   LEDPARPADEO.c
 * Author: EQUIPO
 *
 * Created on 3 de octubre de 2025, 07:56 AM
 */

//Punto C

#include <xc.h>
//#include <pic18f4550.h>

#pragma config FOSC = INTOSCIO_EC   // Oscilador interno (RA6 y RA7 como I/O)
#pragma config WDT = OFF            // Watchdog desactivado
#pragma config LVP = OFF            // Desactiva programación por bajo voltaje (libera RB5)
#pragma config PBADEN = OFF         // PORTB como digital al reset



#define _XTAL_FREQ 8000000        // Frecuencia de 8 MHz 

void ADC_Init(void){
    ADCON1=0x0E;           //AN0 analógico, resto digital
    ADCON2=0xA9;
    ADCON0=0x01;           //Habilita ADC, canal AN0
}
    
unsigned int ADC_Read(unsigned char canal){
    ADCON0 &=0xC5;     //Limpia canal
    ADCON0 |=canal <<3; //Selecciona canal
    __delay_ms(2); //Tiempo de adquisición 
    GO_nDONE=1;   //Inicia conversación
    while(GO_nDONE);  //Espera fin de conversación
    return((ADRESH<<8)+ADRESL);  //Retorna resultado 
}
    
void main(void){
    OSCCON = 0x72;
    TRISAbits.TRISA0=1;   //AN0 como entrada 
    TRISBbits.TRISB0=0;   //LED rojo
    TRISBbits.TRISB1=0;   //LED amarillo
    TRISBbits.TRISB2=0;   //LED verde
        
    LATB=0x00;
    ADC_Init();
        
    unsigned int lectura = 0; 
    float porcentaje = 0; 
        
    while(1){
        lectura=ADC_Read(0); //Lee canal AN0 (0-1023)
        porcentaje = (lectura/1023.0)*100.0;
            
        if(porcentaje<25.0){
            LATBbits.LATB0=1; //LED rojo ON
            LATBbits.LATB1=0; 
            LATBbits.LATB2=0; 
        }
            
        else if(porcentaje>=25.0&&porcentaje<75.0){
            LATBbits.LATB0=0; 
            LATBbits.LATB1=1;   //LED amarillo ON
            LATBbits.LATB2=0; 
        }
                 
        else{
            LATBbits.LATB0=0; 
            LATBbits.LATB1=0;  
            LATBbits.LATB2=1;  //LED verde ON
        }
                    
        __delay_ms(200);  //Pequeña pausa 
                 
                
    }
            
}

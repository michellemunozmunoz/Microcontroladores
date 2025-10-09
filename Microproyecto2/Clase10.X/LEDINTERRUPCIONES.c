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

//Variables globales
volatile unsigned char secuencia=0;   //secuencia (0-3)
volatile unsigned char modo =0;       //modo de velocidad (0-3)
volatile unsigned int velocidad=500;  //retardo incial


//Función para usar variables en los retardos
void delay_ms(unsigned int tiempo){
    while(tiempo--)__delay_ms(1);
}

//Rutina de interrupciones 
void __interrupt() ISR(void){

//Botón RB0: cambiar secuencia
    if (INTCONbits.INT0IF) {
        secuencia++;
    if(secuencia>3)secuencia=0;       //vuelve a 0 después de la última
        INTCONbits.INT0IF = 0;       // Limpiar
    }

//Botón RB1: cambiar velocidad
    if (INTCON3bits.INT1IF) {
    modo++;
    if (modo>3)modo=0;               //reinicia cuando llega al máximo
    
    //cambia la velocidad según el modo 
    if(modo==0)velocidad=800;
    else if(modo==1)velocidad=400;
    else if(modo==2)velocidad=200;
    else velocidad=100;
    
    INTCON3bits.INT1IF = 0;   //Limpia
    } 
}
//Parte principal
void main(void) {
    
    OSCCONbits.IRCF = 0b111;   // IRCF2:0 = 111 ? 8 MHz
    OSCCONbits.SCS = 0b10;     // Usa oscilador interno
    ADCON1 = 0x0F;             // Todos los pines como digitales
    
    //LEDS como salidas (RD0-RD3)
    TRISD=0xF0;                //RD0-RD3 salidas
    LATD=0x00;                 //LEDS apagados

   //Botones como entradas 
    TRISBbits.TRISB0 = 1;      // RB0, cambiar secuencia
    TRISBbits.TRISB1 = 1;      // RB1, cambiar velocidad
    INTCON2bits.RBPU=1;        //Sin reisistencias pull up 
    
    //Configurar interrupciones externas
    INTCON2bits.INTEDG0 = 0;   // Flanco de bajada RB0
    INTCONbits.INT0IF = 0;     // Limpiar INT0
    INTCONbits.INT0IE = 1;     // Habilitar interrupción externa INT0 
    
    INTCON2bits.INTEDG1 = 0;   // Flanco de bajada RB1
    INTCON3bits.INT1IF = 0;    
    INTCON3bits.INT1IE = 1;     
    
    INTCONbits.GIE = 1;        // Habilitar interrupciones globales
    
    //Bucle principal 
    while (1) {
        if(secuencia==0){      //Secuencia 1: LED que corre
            LATD=0b0001;delay_ms(velocidad);
            LATD=0b0010;delay_ms(velocidad);
            LATD=0b0100;delay_ms(velocidad);
            LATD=0b1000;delay_ms(velocidad);
        }
        
        else if(secuencia==1){  //Secuencia 2: ida y vuelta
            LATD=0b0001;delay_ms(velocidad);
            LATD=0b0010;delay_ms(velocidad);
            LATD=0b0100;delay_ms(velocidad);
            LATD=0b1000;delay_ms(velocidad);
            LATD=0b0100;delay_ms(velocidad);
            LATD=0b0010;delay_ms(velocidad);   
        }
        else if(secuencia==2){  //Secuencia 3: par e impar
            LATD=0b0101;delay_ms(velocidad);
            LATD=0b1010;delay_ms(velocidad);
        }
        
        else{                   //Secuencia 4: todos encendidos y apagados
            LATD=0b1111;delay_ms(velocidad);
            LATD=0b0000;delay_ms(velocidad);   
        }
    }
}


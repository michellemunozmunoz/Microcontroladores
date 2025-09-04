;=======================================
;Codigo para PIC18F4550 en Assembler
;Led en RB0 encendido 5s y apagado 2s
;Oscilador interno a 4Mhz
;=======================================
; PIC18F4550 Configuracion de bits

  CONFIG  FOSC = INTOSC_EC      ;Utilizar oscilador interno de 8MHz
  CONFIG  WDT = OFF             ;Desactiva el Watchdog timer 
  CONFIG  PBADEN = OFF          ;Configura los PORTB como digitales
  CONFIG  LVP = OFF             ; Desactiva la programacion a bajo voltage

  
;Incluir definiciones para el PIC18F4550
#include <xc.inc> 
  
  PSECT resetVec, class=CODE, reloc=2 ;Vector de reinicio
  
  ORG 0x00 ;Vector reset
  GOTO Inicio ;Va a Inicio
  
  PSECT main_code, class=CODE, reloc=2 ;Codigo principal
  
  Inicio:
    
    MOVLW 0b01100010 ;valores binarios necesarios para el reloj de 4MHz
    MOVWF OSCCON 
    
    CLRF TRISB  ;PORTB como salida
    CLRF LATB   ;inicio puerto en 0
    GOTO Encendido
    
  Encendido:
    BSF LATB, 0  ;prendo led en RB0
    MOVLW 5 
    MOVWF SegundosContador  ;5 segundos prendido
   
 EncendidoLoop:
    CALL Espera_1s 
    DECFSZ SegundosContador
    GOTO EncendidoLoop 
    GOTO Apagado
   
 Apagado:
    BCF LATB, 0  ;apago led
    MOVLW 2
    MOVWF SegundosContador  ;2 segundos apagado
    
 ApagadoLoop:
    CALL Espera_1s 
    DECFSZ SegundosContador
    GOTO ApagadoLoop
    GOTO Encendido 
; === Espera_1s ====
  Espera_1s:
    MOVLW 5 
    MOVWF ContadorExterno  ;Guarda en la variable ContadorExterno
    
  LoopExterno:
    MOVLW 100 
    MOVWF ContadorMedio  ;Guarda en la variable ContadorMedio
    
  LoopMedio:
    MOVLW 250 
    MOVWF ContadorInterno ;Guarda en la variable ContadorInterno
    
  LoopInterno:
    NOP ;Consume un ciclo
    NOP ;6 NOP para ajustar ciclos
    NOP
    NOP 
    NOP 
    NOP 
    
    DECFSZ ContadorInterno
    GOTO LoopInterno 
    DECFSZ ContadorMedio
    GOTO LoopMedio
    DECFSZ ContadorExterno
    GOTO LoopExterno
    
    RETURN 
    
    PSECT udata
    ContadorExterno: DS 1 
    ContadorMedio: DS 1 
    ContadorInterno: DS 1 
    SegundosContador: DS 1 
 
  END 



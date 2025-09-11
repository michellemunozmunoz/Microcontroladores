;=======================================
; Codigo para PIC18F4550 en Assembler
; LED en RB0 controlado por botón en RB1
; Oscilador interno a 4MHz
;=======================================

  CONFIG  FOSC = INTOSC_EC      
  CONFIG  WDT = OFF             
  CONFIG  PBADEN = OFF          
  CONFIG  LVP = OFF             

#include <xc.inc> 

  PSECT resetVec, class=CODE, reloc=2
  ORG 0x00 
  GOTO Inicio

  PSECT main_code, class=CODE, reloc=2

Inicio:
    ; Configuración del oscilador a 4 MHz
    MOVLW 0b01100010 
    MOVWF OSCCON 
    
    MOVLW 0b00000001 ;RB0 entrada, RB1-RB4 como salidas
    MOVWF TRISB  
    CLRF LATB        ; Limpia puerto B 
    GOTO Secuencia1 ;inicia secuencia 1 
    
    ;-------- Secuencia 1 (0 a 15)---------- 
    
    Secuencia1: 
    MOVLW 0b00000000
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00000010
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00000100
    MOVWF LATB 
    CALL Retardo_200ms 
    
    MOVLW 0b00000110
    MOVWF LATB 
    CALL Retardo_200ms 
    
    MOVLW 0b00001000
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00001010
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00001100
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00001110
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00010000
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00010010
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00010100
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00010110
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00011000
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00011010
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00011100
    MOVWF LATB 
    CALL Retardo_200ms
    
    MOVLW 0b00011110
    MOVWF LATB 
    CALL Retardo_200ms
    
    CLRF LATB 
    CALL Retardo_200ms
    
    CALL RevisaBoton1
    GOTO Secuencia1
    
  ;----------Secuencia 2 (Pares-Impares)
  
    MOVLW 0b00010100   ;pares
    MOVWF LATB 
    CALL Retardo_500ms
    
    MOVLW 0b00001010  ;impares
    MOVWF LATB 
    CALL Retardo_500ms 
    
    CLRF LATB
    CALL Retardo_200ms 
    
    CALL RevisaBoton2
    GOTO Secuencia2 
    
  END

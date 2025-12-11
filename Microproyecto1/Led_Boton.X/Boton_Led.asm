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
  
  ;----------Inicio----------------

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
    GOTO Secuencia2
    
  ;----------Secuencia 2 (Pares-Impares)-------------
  Secuencia2: 
  
    MOVLW 0b00010100   ;pares
    MOVWF LATB 
    CALL Retardo_500ms
    
    MOVLW 0b00001010  ;impares
    MOVWF LATB 
    CALL Retardo_500ms 
    
    CLRF LATB
    CALL Retardo_200ms 
    
    CALL RevisaBoton2
    GOTO Secuencia3 
    
    ;-----------Secuencia 3 (extremo-centro)--------------
    Secuencia3: 
    
    MOVLW 0b00010010 
    MOVWF LATB 
    CALL Retardo_500ms
    
    MOVLW 0b00001100 
    MOVWF LATB 
    CALL Retardo_500ms
    
    MOVLW 0b00011110 
    MOVWF LATB 
    CALL Retardo_500ms
    
    CLRF LATB
    CALL Retardo_500ms 
    
    CALL RevisaBoton3
    GOTO Secuencia1
    
    ;------ Subrutinas para Botón-------
    
    RevisaBoton1:
    BTFSS PORTB,0
    GOTO Secuencia1 ;si está presionado, cambia a secuencia 2
    RETURN 
    
    RevisaBoton2:
    BTFSS PORTB,0
    GOTO Secuencia2 ;si está presionado, cambia a secuencia 3
    RETURN 
    
    RevisaBoton3:
    BTFSS PORTB,0
    GOTO Secuencia3 ;si está presionado, cambia a secuencia 1
    RETURN
    
   ;-----------Subrutinas de retardo---------------
   Retardo_200ms:
    MOVLW 2 
    MOVWF ContadorExterno 
   Loop200a: 
    MOVLW 133 
    MOVWF ContadorMedio
   Loop200b: 
    MOVLW 150 
    MOVWF ContadorInterno
   Loop200c:
    NOP 
    NOP
    NOP 
    DECFSZ ContadorInterno,F
    GOTO Loop200c
    DECFSZ ContadorMedio,F
    GOTO Loop200b
    DECFSZ ContadorExterno,F
    GOTO Loop200a 
    RETURN 
   
   Retardo_500ms:
    MOVLW 2 
    MOVWF ContadorExterno 
   Loop500a: 
    MOVLW 250 
    MOVWF ContadorMedio
   Loop500b: 
    MOVLW 250 
    MOVWF ContadorInterno
   Loop500c:
    NOP 
    NOP
    DECFSZ ContadorInterno,F
    GOTO Loop500c
    DECFSZ ContadorMedio,F
    GOTO Loop500b
    DECFSZ ContadorExterno,F
    GOTO Loop500a 
    RETURN
   
  ;---------Variables-------
  PSECT udata 
    ContadorExterno: DS 1 
    ContadorMedio: DS 1
    ContadorInterno: DS 1
    
  END ;fin 

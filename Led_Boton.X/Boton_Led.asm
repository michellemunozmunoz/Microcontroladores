
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
    
    CLRF LATB        ; Limpia puerto B
    BSF TRISB, 1     ; RB1 como entrada (botón)
    BCF TRISB, 0     ; RB0 como salida (LED)

LoopPrincipal:
    BTFSS PORTB, 1   ; Revisa si el botón en RB1 está presionado (1 = suelto, 0 = presionado si pull-down)
    GOTO BotonPresionado
    BCF LATB, 0      ; Apaga LED si botón no presionado
    GOTO LoopPrincipal

BotonPresionado:
    BSF LATB, 0      ; Enciende LED si botón presionado
    GOTO LoopPrincipal

  END

#ifndef DHT11_H
#define DHT11_H

#include "config.h"

// usamos RA0 para no chocar con I2C
#define DHT11_PIN     PORTAbits.RA0
#define DHT11_TRIS    TRISAbits.TRISA0
#define DHT11_LAT     LATAbits.LATA0

extern uint8_t dht_hum_int, dht_hum_dec, dht_temp_int, dht_temp_dec;

void DHT11_Start(void);
uint8_t DHT11_CheckResponse(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadFrame(void);   // lee los 5 bytes y los guarda

#endif


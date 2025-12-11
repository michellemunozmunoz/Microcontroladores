#ifndef DHT11_H
#define DHT11_H

#include <xc.h>
#include "config.h"

// Pin definitions for DHT11
#define DHT11_PIN LATBbits.LATB4
#define DHT11_DIR TRISBbits.TRISB4
#define DHT11_READ PORTBbits.RB4

// Function declarations
void DHT11_Init(void);
unsigned char DHT11_Read(float *temperature, float *humidity);

#endif
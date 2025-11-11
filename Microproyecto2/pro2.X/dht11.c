#include "dht11.h"
#include <stdio.h>   // si luego quieres imprimir en LCD
#include <stdint.h>

uint8_t dht_hum_int, dht_hum_dec, dht_temp_int, dht_temp_dec, dht_checksum;

static uint8_t waitForState(uint8_t state, uint16_t timeout_us)
{
    while (((DHT11_PIN & 1) != state)) {
        if (timeout_us-- == 0) return 0;
        __delay_us(1);
    }
    return 1;
}

void DHT11_Start(void)
{
    DHT11_TRIS = 0;       // salida
    DHT11_LAT  = 0;       // bajo
    __delay_ms(18);       // start signal
    DHT11_LAT  = 1;       // soltar
    __delay_us(20);
    DHT11_TRIS = 1;       // entrada
}

uint8_t DHT11_CheckResponse(void)
{
    if (!waitForState(0, 200)) return 0;
    if (!waitForState(1, 200)) return 0;
    return 1;
}

uint8_t DHT11_ReadByte(void)
{
    uint8_t i, data = 0;
    for (i = 0; i < 8; i++) {
        if (!waitForState(0, 200)) return 0xFF;
        if (!waitForState(1, 400)) return 0xFF;
        __delay_us(40);
        data <<= 1;
        if (DHT11_PIN & 1) data |= 1;
    }
    return data;
}

uint8_t DHT11_ReadFrame(void)
{
    DHT11_Start();
    if (!DHT11_CheckResponse()) return 0;

    uint8_t h1 = DHT11_ReadByte();
    if (h1 == 0xFF) return 0;
    uint8_t h2 = DHT11_ReadByte();
    if (h2 == 0xFF) return 0;
    uint8_t t1 = DHT11_ReadByte();
    if (t1 == 0xFF) return 0;
    uint8_t t2 = DHT11_ReadByte();
    if (t2 == 0xFF) return 0;
    uint8_t cs = DHT11_ReadByte();
    if (cs == 0xFF) return 0;

    if ( (uint8_t)(h1 + h2 + t1 + t2) != cs ) {
        return 0;
    }

    dht_hum_int  = h1;
    dht_hum_dec  = h2;
    dht_temp_int = t1;
    dht_temp_dec = t2;
    dht_checksum = cs;
    return 1;
}


#include "dht11.h"

void DHT11_Init(void) {
    DHT11_DIR = 1; // Initially as input
}

unsigned char DHT11_Read(float *temperature, float *humidity) {
    unsigned char data[5] = {0, 0, 0, 0, 0};
    unsigned char i, j;
    
    // Send start signal
    DHT11_DIR = 0; // Set as output
    DHT11_PIN = 0;
    __delay_ms(18);
    DHT11_PIN = 1;
    __delay_us(30);
    DHT11_DIR = 1; // Set as input
    __delay_us(40);
    
    // Wait for DHT response
    if (DHT11_READ) return 0;
    __delay_us(80);
    if (!DHT11_READ) return 0;
    __delay_us(80);
    
    // Read 40 bits of data
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            while (!DHT11_READ); // Wait for high
            __delay_us(30);
            
            if (DHT11_READ) {
                data[i] |= (1 << (7 - j));
                while (DHT11_READ); // Wait for low
            }
        }
    }
    
    // Verify checksum
    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        return 0;
    }
    
    // Convert to temperature and humidity
    *humidity = (float)data[0];
    *temperature = (float)data[2];
    
    return 1;
}
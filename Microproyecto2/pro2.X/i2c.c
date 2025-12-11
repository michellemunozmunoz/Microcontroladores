#include "i2c.h"

void I2C_Init(void) {
    SSPCON1 = 0x28;
    SSPCON2 = 0x00;
    SSPADD  = ((_XTAL_FREQ / 4) / 100000) - 1;
    SSPSTAT = 0x00;

    TRISBbits.TRISB0 = 1; // SDA
    TRISBbits.TRISB1 = 1; // SCL
}

void I2C_Start(void) {
    SSPCON2bits.SEN = 1;
    while (SSPCON2bits.SEN);
}

void I2C_Stop(void) {
    SSPCON2bits.PEN = 1;
    while (SSPCON2bits.PEN);
}

void I2C_Write(unsigned char data) {
    SSPBUF = data;
    while (BF);
    while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));
}

unsigned char I2C_Read(unsigned char ack) {
    unsigned char data;
    SSPCON2bits.RCEN = 1;
    while (!SSPSTATbits.BF);
    data = SSPBUF;
    SSPCON2bits.ACKDT = (ack) ? 0 : 1;
    SSPCON2bits.ACKEN = 1;
    while (SSPCON2bits.ACKEN);
    return data;
}

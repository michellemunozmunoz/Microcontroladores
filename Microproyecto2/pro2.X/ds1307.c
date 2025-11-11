#include "ds1307.h"

#define DS1307_ADDRESS 0xD0

static unsigned char decToBcd(unsigned char val) {
    return ((val / 10 * 16) + (val % 10));
}

static unsigned char bcdToDec(unsigned char val) {
    return ((val / 16 * 10) + (val % 16));
}

void DS1307_Init(void) {
    I2C_Init();
}

void DS1307_SetTime(unsigned char h, unsigned char m, unsigned char s) {
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x00);
    I2C_Write(decToBcd(s & 0x7F));
    I2C_Write(decToBcd(m));
    I2C_Write(decToBcd(h));
    I2C_Stop();
}

void DS1307_GetTime(unsigned char *h, unsigned char *m, unsigned char *s) {
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x00);
    I2C_Start();
    I2C_Write(DS1307_ADDRESS | 1);
    *s = bcdToDec(I2C_Read(1));
    *m = bcdToDec(I2C_Read(1));
    *h = bcdToDec(I2C_Read(0));
    I2C_Stop();
}

void DS1307_SetDate(unsigned char d, unsigned char mth, unsigned char y) {
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x04);
    I2C_Write(decToBcd(d));
    I2C_Write(decToBcd(mth));
    I2C_Write(decToBcd(y));
    I2C_Stop();
}

void DS1307_GetDate(unsigned char *d, unsigned char *mth, unsigned char *y) {
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(0x04);
    I2C_Start();
    I2C_Write(DS1307_ADDRESS | 1);
    *d   = bcdToDec(I2C_Read(1));
    *mth = bcdToDec(I2C_Read(1));
    *y   = bcdToDec(I2C_Read(0));
    I2C_Stop();
}

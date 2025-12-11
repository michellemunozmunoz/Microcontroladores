#ifndef DS1307_H
#define DS1307_H

#include "config.h"
#include "i2c.h"

void DS1307_Init(void);
void DS1307_SetTime(unsigned char h, unsigned char m, unsigned char s);
void DS1307_GetTime(unsigned char *h, unsigned char *m, unsigned char *s);
void DS1307_SetDate(unsigned char d, unsigned char mth, unsigned char y);
void DS1307_GetDate(unsigned char *d, unsigned char *mth, unsigned char *y);

#endif

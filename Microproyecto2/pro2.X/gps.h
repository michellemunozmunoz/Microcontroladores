#ifndef GPS_H
#define GPS_H

#include "config.h"

void GPS_UART_Init(long baudrate);
void GPS_Task(void);
unsigned char GPS_HasFix(void);
float GPS_GetLat(void);
float GPS_GetLon(void);
unsigned long GPS_GetUTC(void);

#endif

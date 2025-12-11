#include "gps.h"
#include <string.h>   // strlen, strcpy, strtok, strstr
#include <stdlib.h>   // atof, atol

static char nmea_buffer[256];
static unsigned int nmea_index = 0;
static unsigned char nmea_started = 0;

static char lat_raw[16] = "", lon_raw[16] = "", time_raw[16] = "";
static char ns = 'N', ew = 'E';
static float lat = 0.0, lon = 0.0;
static unsigned long time_utc = 0;
static unsigned char gps_valid = 0;

static float GPS_ConvertToDecimal(const char *value) {
    if (value == NULL || strlen(value) < 4) return 0.0;
    float val = atof(value);
    int grados = (int)(val / 100);
    float minutos = val - (grados * 100.0);
    return grados + (minutos / 60.0);
}

static unsigned char GPS_ParseGPGGA(char *sentence) {
    if (strstr(sentence, "$GPGGA") == NULL) return 0;

    char temp[256];
    strcpy(temp, sentence);

    char *token;
    int field = 0;

    strcpy(lat_raw, "");
    strcpy(lon_raw, "");
    strcpy(time_raw, "");
    ns = 'N'; ew = 'E';

    token = strtok(temp, ",");
    while (token != NULL) {
        field++;
        switch (field) {
            case 2: strcpy(time_raw, token); break;
            case 3: strcpy(lat_raw, token); break;
            case 4: ns = token[0]; break;
            case 5: strcpy(lon_raw, token); break;
            case 6: ew = token[0]; break;
        }
        token = strtok(NULL, ",");
    }

    if (strlen(lat_raw) < 4 || strlen(lon_raw) < 4)
        return 0;

    lat = GPS_ConvertToDecimal(lat_raw);
    lon = GPS_ConvertToDecimal(lon_raw);
    time_utc = atol(time_raw);

    if (ns == 'S') lat = -lat;
    if (ew == 'W') lon = -lon;

    return 1;
}

void GPS_UART_Init(long baudrate) {
    TRISCbits.TRISC6 = 1; // TX
    TRISCbits.TRISC7 = 1; // RX

    SPBRG = (unsigned char)((_XTAL_FREQ / (64UL * baudrate)) - 1);
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 0;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 0;   // solo RX
    RCSTAbits.CREN = 1;
    PIR1bits.RCIF = 0;
}

static unsigned char UART_Available(void) {
    return PIR1bits.RCIF;
}

static char UART_Read(void) {
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        __delay_us(10);
        RCSTAbits.CREN = 1;
    }
    return RCREG;
}

void GPS_Task(void) {
    if (UART_Available()) {
        char c = UART_Read();

        if (c == '$') {
            nmea_started = 1;
            nmea_index = 0;
            nmea_buffer[nmea_index++] = c;
        } else if (nmea_started) {
            if (c == '\n' || c == '\r') {
                nmea_buffer[nmea_index] = '\0';
                gps_valid = GPS_ParseGPGGA(nmea_buffer);
                nmea_started = 0;
                nmea_index = 0;
            } else {
                if (nmea_index < sizeof(nmea_buffer) - 1) {
                    nmea_buffer[nmea_index++] = c;
                }
            }
        }
    }
}

unsigned char GPS_HasFix(void) { return gps_valid; }
float GPS_GetLat(void) { return lat; }
float GPS_GetLon(void) { return lon; }
unsigned long GPS_GetUTC(void) { return time_utc; }

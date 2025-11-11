#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "LCD.h"
#include "ds1307.h"
#include "dht11.h"

// ---------- CONFIGURACIÓN UART PARA GPS ----------
void UART_Init(long baudrate) {
    TRISCbits.TRISC6 = 1;  // TX como entrada
    TRISCbits.TRISC7 = 1;  // RX como entrada
    
    SPBRG = (unsigned char)((_XTAL_FREQ / (64UL * baudrate)) - 1);
    
    TXSTAbits.SYNC = 0;    // Modo asíncrono
    TXSTAbits.BRGH = 0;    // Low speed
    RCSTAbits.SPEN = 1;    // Habilitar puerto serial
    TXSTAbits.TXEN = 0;    // Solo recepción
    RCSTAbits.CREN = 1;    // Habilitar recepción continua
    
    PIR1bits.RCIF = 0;
}

unsigned char UART_Available(void) {
    return PIR1bits.RCIF;
}

char UART_Read(void) {
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        __delay_us(10);
        RCSTAbits.CREN = 1;
    }
    return RCREG;
}

// ---------- VARIABLES GPS ----------
char nmea_buffer[128];
char lat_raw[12] = "", lon_raw[12] = "", alt_raw[10] = "", time_raw[10] = "";
char ns = 'N', ew = 'E';
float lat = 0.0, lon = 0.0, alt = 0.0;
unsigned long time_utc = 0;
unsigned char gps_valid = 0;

// ---------- VARIABLES DHT11 ----------
float temperature = 0.0;
float humidity = 0.0;
unsigned char dht_valid = 0;

// ---------- CONVERSIÓN COORDENADAS ----------
float ConvertToDecimal(const char *value) {
    if (value == NULL || strlen(value) < 4) return 0.0;
    
    float val = atof(value);
    int grados = (int)(val / 100);
    float minutos = val - (grados * 100.0);
    return grados + (minutos / 60.0);
}

// ---------- PARSING GPS ----------
unsigned char ParseGPGGA(char *sentence) {
    if (strstr(sentence, "$GPGGA") == NULL) {
        return 0;
    }
    
    strcpy(lat_raw, "");
    strcpy(lon_raw, "");
    strcpy(alt_raw, "");
    strcpy(time_raw, "");
    ns = 'N';
    ew = 'E';
    
    char temp[128];
    strcpy(temp, sentence);
    
    char *token;
    int field = 0;
    
    token = strtok(temp, ",");
    
    while (token != NULL && field < 15) {
        field++;
        
        switch (field) {
            case 1:  // $GPGGA
                break;
            case 2:  // Tiempo UTC
                if(strlen(token) >= 6) strcpy(time_raw, token);
                break;
            case 3:  // Latitud
                if(strlen(token) >= 4) strcpy(lat_raw, token);
                break;
            case 4:  // N/S
                if(strlen(token) == 1) ns = token[0];
                break;
            case 5:  // Longitud
                if(strlen(token) >= 4) strcpy(lon_raw, token);
                break;
            case 6:  // E/W
                if(strlen(token) == 1) ew = token[0];
                break;
            case 7:  // Fix quality
                break;
            case 8:  // Número de satélites
                break;
            case 9:  // HDOP
                break;
            case 10: // Altitud
                if(strlen(token) > 0) strcpy(alt_raw, token);
                break;
        }
        
        token = strtok(NULL, ",");
    }
    
    if (strlen(lat_raw) < 4 || strlen(lon_raw) < 4) {
        return 0;
    }
    
    lat = ConvertToDecimal(lat_raw);
    lon = ConvertToDecimal(lon_raw);
    alt = atof(alt_raw);
    time_utc = atol(time_raw);
    
    if (ns == 'S') lat = -lat;
    if (ew == 'W') lon = -lon;
    
    return 1;
}

// ---------- FUNCIONES DE VISUALIZACIÓN ----------
void DisplayRTCData(void) {
    unsigned char h, m, s, d, mt, y;
    char buffer[17];
    
    DS1307_GetTime(&h, &m, &s);
    DS1307_GetDate(&d, &mt, &y);
    
    // Línea 1: Fecha
    sprintf(buffer, "Fecha:%02d/%02d/20%02d", d, mt, y);
    LCD_String_xy(0, 0, buffer);
    
    // Línea 2: Hora
    sprintf(buffer, "Hora: %02d:%02d:%02d", h, m, s);
    LCD_String_xy(1, 0, buffer);
}

void DisplayDHT11Data(void) {
    char buffer[17];
    
    if(!dht_valid) {
        LCD_Clear();
        LCD_String_xy(0, 0, " Error Sensor");
        LCD_String_xy(1, 0, "   DHT11");
        return;
    }
    
    LCD_Clear();
    
    // Línea 1: Temperatura
    sprintf(buffer, "Temp:   %2.1f%cC", temperature, 223); // 223 = ° símbolo
    LCD_String_xy(0, 0, buffer);
    
    // Línea 2: Humedad
    sprintf(buffer, "Hum:    %2.1f%%", humidity);
    LCD_String_xy(1, 0, buffer);
}

void DisplayGPSData(void) {
    char buffer[17];
    static unsigned char screen = 0;
    
    if(!gps_valid) {
        LCD_Clear();
        LCD_String_xy(0, 0, "  SIN SEÑAL GPS");
        LCD_String_xy(1, 0, "  Buscando...");
        return;
    }
    
    if(screen == 0) {
        // Pantalla 1: Coordenadas
        LCD_Clear();
        
        // Línea 1: Latitud
        if(lat >= 0)
            sprintf(buffer, "Lat:%9.5fN", lat);
        else
            sprintf(buffer, "Lat:%9.5fS", -lat);
        LCD_String_xy(0, 0, buffer);
        
        // Línea 2: Longitud
        if(lon >= 0)
            sprintf(buffer, "Lon:%9.5fE", lon);
        else
            sprintf(buffer, "Lon:%9.5fW", -lon);
        LCD_String_xy(1, 0, buffer);
        
        screen = 1;
    } else {
        // Pantalla 2: Altitud y Tiempo
        LCD_Clear();
        
        // Línea 1: Altitud
        sprintf(buffer, "Alt:%8.1f m", alt);
        LCD_String_xy(0, 0, buffer);
        
        // Línea 2: Tiempo UTC
        if(time_utc > 0) {
            unsigned int hours = time_utc / 10000;
            unsigned int minutes = (time_utc % 10000) / 100;
            unsigned int seconds = time_utc % 100;
            sprintf(buffer, "UTC %02d:%02d:%02d", hours, minutes, seconds);
        } else {
            sprintf(buffer, "UTC --:--:--");
        }
        LCD_String_xy(1, 0, buffer);
        
        screen = 0;
    }
}

// ---------- FUNCIÓN PRINCIPAL ----------
void main(void) {
    unsigned char display_mode = 0; // 0=RTC, 1=DHT11, 2=GPS
    unsigned long display_timer = 0;
    unsigned long mode_switch_time = 5000; // 5 segundos por modo
    unsigned long dht_read_timer = 0;
    unsigned long gps_display_timer = 0;
    
    // Configuración del sistema
    OSCCONbits.IRCF = 0b111; // 8MHz
    ADCON1 = 0x0F;
    CMCON = 0x07;
    
    // Inicializar periféricos
    LCD_Init();
    DS1307_Init();
    DHT11_Init();
    UART_Init(9600);
    
    // Configurar RTC si no está funcionando
    if (!DS1307_IsRunning()) {
        LCD_Clear();
        LCD_String_xy(0, 0, "Config. RTC...");
        DS1307_SetTime(12, 0, 0);
        DS1307_SetDate(1, 1, 25); // 1 de enero de 2025
        __delay_ms(1000);
    }
    
    LCD_Clear();
    LCD_String_xy(0, 0, "Sistema Iniciado");
    LCD_String_xy(1, 0, "RTC+DHT11+GPS");
    __delay_ms(2000);
    
    // Variables para recepción GPS
    unsigned int index = 0;
    char received_char;
    unsigned char sentence_started = 0;
    unsigned long gps_timeout = 0;
    
    while(1) {
        // ---------- LECTURA DHT11 (cada 2 segundos) ----------
        dht_read_timer++;
        if(dht_read_timer >= 2000) {
            dht_valid = DHT11_Read(&temperature, &humidity);
            dht_read_timer = 0;
        }
        
        // ---------- PROCESAMIENTO GPS ----------
        if(UART_Available()) {
            received_char = UART_Read();
            
            if(received_char == '$') {
                index = 0;
                sentence_started = 1;
                nmea_buffer[index++] = received_char;
            }
            else if(sentence_started) {
                if(received_char == '\n') {
                    nmea_buffer[index] = '\0';
                    
                    if(strstr(nmea_buffer, "$GPGGA") != NULL) {
                        gps_valid = ParseGPGGA(nmea_buffer);
                    }
                    
                    sentence_started = 0;
                    index = 0;
                    gps_timeout = 0;
                }
                else if(index < sizeof(nmea_buffer) - 1) {
                    nmea_buffer[index++] = received_char;
                }
            }
        }
        else {
            __delay_ms(1);
            gps_timeout++;
            
            // Timeout GPS después de 10 segundos sin datos
            if(gps_timeout > 10000) {
                gps_valid = 0;
                gps_timeout = 0;
            }
        }
        
        // ---------- CONTROL DE VISUALIZACIÓN ----------
        display_timer++;
        gps_display_timer++;
        
        // Cambiar modo cada 5 segundos
        if(display_timer >= mode_switch_time) {
            display_mode++;
            if(display_mode > 2) display_mode = 0;
            display_timer = 0;
            LCD_Clear();
        }
        
        // Mostrar datos según el modo actual
        switch(display_mode) {
            case 0: // Modo RTC
                if(display_timer == 0) {
                    DisplayRTCData();
                }
                // Actualizar display RTC cada segundo
                if(display_timer % 1000 == 0) {
                    DisplayRTCData();
                }
                break;
                
            case 1: // Modo DHT11
                if(display_timer == 0) {
                    DisplayDHT11Data();
                }
                break;
                
            case 2: // Modo GPS
                if(display_timer == 0) {
                    DisplayGPSData();
                }
                // Alternar pantallas GPS cada 3 segundos
                if(gps_display_timer >= 3000) {
                    DisplayGPSData();
                    gps_display_timer = 0;
                }
                break;
        }
        
        __delay_ms(1);
    }
}
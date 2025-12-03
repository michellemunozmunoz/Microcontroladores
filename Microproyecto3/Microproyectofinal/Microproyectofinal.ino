// Incluye las bibliotecas necesarias
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"

// -------- CONFIGURACIÓN DE LA RED WiFi --------
const char* ssid = "Michelle";
const char* password = "Michelle1234567890";

// -------- CONFIGURACIÓN DE PINES CORREGIDA --------
#define DHTPIN 2           // GPIO2 - DHT11 (seguro para WiFi)
#define DHTTYPE DHT11
#define MQ135_PIN 3        // GPIO3 - MQ-135 (seguro para WiFi)  
#define LDR_PIN 1          // GPIO1 - LDR (seguro para WiFi)

// ----- PINES I2C -----
#define I2C_SDA 4         // SDA en GPIO4
#define I2C_SCL 5         // SCL en GPIO5

// ----- CONFIGURACIÓN DE LA OLED -----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----- RTC -----
RTC_DS3231 rtc;

// ----- DHT11 -----
DHT dht(DHTPIN, DHTTYPE);

// -------- CREACIÓN DEL SERVIDOR WEB --------
WebServer server(80);

// -------- VARIABLES GLOBALES --------
float tempMax = -1000, tempMin = 1000;
float humMax = -1000, humMin = 1000;
float airQualityMax = -1000, airQualityMin = 1000;
float temp = 0, hum = 0, airQuality = 0;
bool systemEnabled = true;
String currentTime = "--:--:--";
String currentDate = "--/--/--";
int lightLevel = 0;
bool isDayMode = true;
bool showPausedMessage = false;
unsigned long pausedMessageTime = 0;
bool gasAlert = false;
bool ventilationAlert = false;
bool evacuationAlert = false;

// -------- CONTENIDO HTML CON ALERTAS DE GAS --------
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Sistema Climático</title>
  <style>
    html { 
      font-family: Arial; 
      text-align: center; 
      transition: background-color 0.5s, color 0.5s;
    }
    
    .day-mode {
      background-color: #f0f8ff;
      color: #333;
    }
    
    .night-mode {
      background-color: #1a1a2e;
      color: #e6e6e6;
    }
    
    h2 { font-size: 2.2rem; margin-bottom: 20px; }
    .datetime { font-size: 1.2rem; margin-bottom: 20px; }
    
    .container { 
      display: flex; 
      justify-content: center; 
      gap: 20px; 
      flex-wrap: wrap; 
      margin-bottom: 30px;
    }
    
    .sensor-block { 
      display: flex; 
      flex-direction: column; 
      align-items: center; 
      margin-bottom: 25px;
      padding: 20px;
      border-radius: 15px;
      transition: all 0.5s;
      min-width: 250px;
    }
    
    .day-mode .sensor-block {
      background-color: #ffffff;
      box-shadow: 0 4px 15px rgba(0,0,0,0.1);
    }
    
    .night-mode .sensor-block {
      background-color: #16213e;
      box-shadow: 0 4px 15px rgba(255,255,255,0.1);
    }
    
    .bar-wrapper { 
      display: flex; 
      flex-direction: row; 
      align-items: flex-end; 
      gap: 8px; 
      margin: 15px 0;
    }
    
    .bar-container {
      position: relative; 
      width: 50px; 
      height: 200px;
      background: linear-gradient(to top, #0000ff, #00ffff, #00ff00, #ffff00, #ff0000);
      border-radius: 25px; 
      overflow: hidden; 
      border: 2px solid #999;
    }
    
    .bar-mask {
      position: absolute; 
      top: 0; 
      width: 100%;
      background: rgba(0, 0, 0, 0.5);
      transition: height 0.5s; 
      z-index: 1;
    }
    
    .night-mode .bar-mask {
      background: rgba(255, 255, 255, 0.3);
    }
    
    .scale {
      display: flex; 
      flex-direction: column; 
      justify-content: space-between;
      height: 200px; 
      font-size: 0.8rem;
    }
    
    .label { 
      margin-top: 10px; 
      font-size: 1.1rem; 
      font-weight: bold; 
    }
    
    .value { 
      font-size: 1.8rem; 
      margin-top: 10px; 
      font-weight: bold;
    }
    
    .minmax { 
      font-size: 0.9rem; 
      margin-top: 8px; 
    }
    
    .day-mode .minmax {
      color: #666;
    }
    
    .night-mode .minmax {
      color: #ccc;
    }
    
    .control-panel {
      margin: 20px 0;
      padding: 25px;
      border-radius: 15px;
      transition: all 0.5s;
    }
    
    .day-mode .control-panel {
      background-color: #e8f4fd;
    }
    
    .night-mode .control-panel {
      background-color: #0f3460;
    }
    
    .btn {
      padding: 12px 25px;
      font-size: 1.1rem;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      margin: 8px;
      transition: all 0.3s;
      font-weight: bold;
    }
    
    .btn-enable {
      background: linear-gradient(45deg, #4CAF50, #45a049);
      color: white;
    }
    
    .btn-disable {
      background: linear-gradient(45deg, #f44336, #da190b);
      color: white;
    }
    
    .btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(0,0,0,0.2);
    }
    
    .status {
      font-size: 1.2rem;
      margin: 15px 0;
      padding: 12px;
      border-radius: 8px;
      font-weight: bold;
    }
    
    .status-enabled {
      background: #d4edda;
      color: #155724;
      border: 2px solid #c3e6cb;
    }
    
    .status-disabled {
      background: #f8d7da;
      color: #721c24;
      border: 2px solid #f5c6cb;
    }
    
    .air-quality-indicator {
      width: 150px;
      height: 25px;
      border-radius: 12px;
      margin: 12px auto;
      transition: background-color 0.5s;
      display: flex;
      align-items: center;
      justify-content: center;
      color: white;
      font-weight: bold;
      font-size: 0.9rem;
    }
    
    .air-excellent { background: linear-gradient(45deg, #00b894, #00a085); }
    .air-good { background: linear-gradient(45deg, #4CAF50, #45a049); }
    .air-moderate { background: linear-gradient(45deg, #FFC107, #ff9800); }
    .air-poor { background: linear-gradient(45deg, #ff7675, #fd79a8); }
    .air-very-poor { background: linear-gradient(45deg, #e17055, #d63031); }
    .air-dangerous { 
      background: linear-gradient(45deg, #8B0000, #FF0000);
      animation: pulse 1s infinite;
    }
    
    .mode-indicator {
      margin: 10px 0;
      padding: 10px;
      border-radius: 8px;
      font-weight: bold;
    }
    
    .day-indicator {
      background: #fff3cd;
      color: #856404;
      border: 1px solid #ffeaa7;
    }
    
    .night-indicator {
      background: #cce7ff;
      color: #004085;
      border: 1px solid #b3d7ff;
    }
    
    .alert-banner {
      background: linear-gradient(45deg, #ff0000, #ff6b6b);
      color: white;
      padding: 15px;
      margin: 20px 0;
      border-radius: 10px;
      animation: blink 1s infinite;
      font-weight: bold;
      font-size: 1.2rem;
    }
    
    .warning-banner {
      background: linear-gradient(45deg, #ff9800, #ff5722);
      color: white;
      padding: 15px;
      margin: 20px 0;
      border-radius: 10px;
      animation: blink 2s infinite;
      font-weight: bold;
      font-size: 1.2rem;
    }
    
    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.7; }
      100% { opacity: 1; }
    }
    
    @keyframes pulse {
      0% { transform: scale(1); }
      50% { transform: scale(1.05); }
      100% { transform: scale(1); }
    }
    
    .alert-hidden {
      display: none;
    }
  </style>
</head>
<body class="day-mode" id="body">
  <h2>🌤 Sistema de Monitoreo Climático ESP32</h2>
  
  <!-- ALERTA DE EVACUACIÓN (≥3000 PPM) -->
  <div id="evacuation-alert" class="alert-banner alert-hidden">
    🚨 ALERTA PELIGRO! EVACUAR O VENTILAR INMEDIATAMENTE - AIRE PELIGROSO 🚨
  </div>
  
  <!-- ALERTA DE VENTILACIÓN (2000-2999 PPM) -->
  <div id="ventilation-alert" class="warning-banner alert-hidden">
    ⚠️ ADVERTENCIA! VENTILAR EL ÁREA - CALIDAD DEL AIRE MUY MALA ⚠️
  </div>
  
  <div class="datetime">
    <div id="datetime">--/--/-- --:--:--</div>
    <div id="light-info">💡 Nivel de luz: --</div>
    <div id="mode-indicator" class="mode-indicator day-indicator">☀️ Día (Automático)</div>
  </div>
  
  <div class="control-panel">
    <h3>🎛️ Control del Sistema</h3>
    <button class="btn btn-enable" onclick="toggleSystem(true)">🟢 ACTIVAR SISTEMA</button>
    <button class="btn btn-disable" onclick="toggleSystem(false)">🔴 DESACTIVAR SISTEMA</button>
    
    <div id="system-status" class="status status-enabled">✅ SISTEMA ACTIVADO</div>
  </div>
  
  <div class="container">
    <!-- Termómetro de Temperatura -->
    <div class="sensor-block">
      <div class="bar-wrapper">
        <div class="bar-container">
          <div class="bar-mask" id="temperature-mask" style="height: 100%;"></div>
        </div>
        <div class="scale">
          <div>50</div><div>40</div><div>30</div><div>20</div><div>10</div><div>0</div>
        </div>
      </div>
      <div class="label">🌡 Temperatura</div>
      <div class="value"><span id="temperature">--</span>°C</div>
      <div class="minmax">📈 Máx: <span id="temperature-max">--</span>°C | 📉 Mín: <span id="temperature-min">--</span>°C</div>
    </div>
    
    <!-- Termómetro de Humedad -->
    <div class="sensor-block">
      <div class="bar-wrapper">
        <div class="bar-container">
          <div class="bar-mask" id="humidity-mask" style="height: 100%;"></div>
        </div>
        <div class="scale">
          <div>100</div><div>80</div><div>60</div><div>40</div><div>20</div><div>0</div>
        </div>
      </div>
      <div class="label">💧 Humedad</div>
      <div class="value"><span id="humidity">--</span>%</div>
      <div class="minmax">📈 Máx: <span id="humidity-max">--</span>% | 📉 Mín: <span id="humidity-min">--</span>%</div>
    </div>
    
    <!-- Indicador de Calidad del Aire -->
    <div class="sensor-block">
      <div class="label">🌫 Calidad del Aire</div>
      <div class="value"><span id="air-quality">--</span> ppm</div>
      <div id="air-quality-indicator" class="air-quality-indicator air-excellent">EXCELENTE</div>
      <div class="minmax">📈 Máx: <span id="air-quality-max">--</span> ppm | 📉 Mín: <span id="air-quality-min">--</span> ppm</div>
    </div>
  </div>
  
  <script>
    function updateData() {
      fetch('/sensors')
        .then(response => {
          if (!response.ok) throw new Error('Error del servidor');
          return response.json();
        })
        .then(data => {
          // Actualizar datos de sensores
          document.getElementById('temperature').textContent = data.temp.toFixed(1);
          document.getElementById('temperature-max').textContent = data.tempMax.toFixed(1);
          document.getElementById('temperature-min').textContent = data.tempMin.toFixed(1);
          
          document.getElementById('humidity').textContent = data.hum.toFixed(1);
          document.getElementById('humidity-max').textContent = data.humMax.toFixed(1);
          document.getElementById('humidity-min').textContent = data.humMin.toFixed(1);
          
          document.getElementById('air-quality').textContent = data.airQuality.toFixed(1);
          document.getElementById('air-quality-max').textContent = data.airQualityMax.toFixed(1);
          document.getElementById('air-quality-min').textContent = data.airQualityMin.toFixed(1);
          
          // Actualizar barras
          document.getElementById('temperature-mask').style.height = (100 - (data.temp * 2)) + '%';
          document.getElementById('humidity-mask').style.height = (100 - data.hum) + '%';
          
          // Actualizar indicador de calidad del aire con nuevos rangos
          const airIndicator = document.getElementById('air-quality-indicator');
          if (data.airQuality < 300) {
            airIndicator.className = 'air-quality-indicator air-excellent';
            airIndicator.textContent = 'EXCELENTE';
          } else if (data.airQuality < 600) {
            airIndicator.className = 'air-quality-indicator air-good';
            airIndicator.textContent = 'BUENA';
          } else if (data.airQuality < 1000) {
            airIndicator.className = 'air-quality-indicator air-moderate';
            airIndicator.textContent = 'MODERADA';
          } else if (data.airQuality < 2000) {
            airIndicator.className = 'air-quality-indicator air-poor';
            airIndicator.textContent = 'MALA';
          } else if (data.airQuality < 3000) {
            airIndicator.className = 'air-quality-indicator air-very-poor';
            airIndicator.textContent = 'MUY MALA';
          } else {
            airIndicator.className = 'air-quality-indicator air-dangerous';
            airIndicator.textContent = 'PELIGROSA';
          }
          
          // Actualizar fecha y hora
          document.getElementById('datetime').textContent = data.date + ' ' + data.time;
          
          // Actualizar información de luz
          document.getElementById('light-info').innerHTML = '💡 Nivel de luz: ' + data.lightLevel;
          
          // Actualizar modo día/noche automático
          const modeIndicator = document.getElementById('mode-indicator');
          if (data.isDayMode) {
            document.getElementById('body').className = 'day-mode';
            modeIndicator.className = 'mode-indicator day-indicator';
            modeIndicator.textContent = '☀️ Día (Automático)';
          } else {
            document.getElementById('body').className = 'night-mode';
            modeIndicator.className = 'mode-indicator night-indicator';
            modeIndicator.textContent = '🌙 Noche (Automático)';
          }
          
          // Actualizar estado del sistema
          const statusElement = document.getElementById('system-status');
          statusElement.textContent = data.systemEnabled ? 
            '✅ SISTEMA ACTIVADO' : '❌ SISTEMA DESACTIVADO';
          statusElement.className = data.systemEnabled ? 
            'status status-enabled' : 'status status-disabled';
          
          // Mostrar/ocultar alertas escalonadas
          const evacuationAlert = document.getElementById('evacuation-alert');
          const ventilationAlert = document.getElementById('ventilation-alert');
          
          if (data.airQuality >= 3000) {
            evacuationAlert.classList.remove('alert-hidden');
            ventilationAlert.classList.add('alert-hidden');
          } else if (data.airQuality >= 2000) {
            evacuationAlert.classList.add('alert-hidden');
            ventilationAlert.classList.remove('alert-hidden');
          } else {
            evacuationAlert.classList.add('alert-hidden');
            ventilationAlert.classList.add('alert-hidden');
          }
        })
        .catch(error => {
          console.error('Error:', error);
          document.getElementById('system-status').textContent = '❌ ERROR DE CONEXIÓN';
          document.getElementById('system-status').className = 'status status-disabled';
        });
    }
    
    function toggleSystem(enable) {
      fetch('/control?enable=' + (enable ? '1' : '0'))
        .then(response => response.text())
        .then(() => updateData());
    }
    
    setInterval(updateData, 2000);
    window.onload = updateData;
  </script>
</body></html>
)rawliteral";

// -------- FUNCIÓN PARA LEER CALIDAD DEL AIRE --------
float readAirQuality() {
  int sensorValue = analogRead(MQ135_PIN);
  float voltage = sensorValue * (3.3 / 4095.0);
  float ppm = (voltage - 0.1) * 1000.0;
  if (ppm < 0) ppm = 0;
  
  // Sistema de alertas escalonado
  if (ppm >= 3000) {
    evacuationAlert = true;
    ventilationAlert = true;
  } else if (ppm >= 2000) {
    evacuationAlert = false;
    ventilationAlert = true;
  } else {
    evacuationAlert = false;
    ventilationAlert = false;
  }
  
  return ppm;
}

// -------- FUNCIÓN PARA LEER SENSOR DE LUZ --------
int readLightSensor() {
  return analogRead(LDR_PIN);
}

// -------- FUNCIÓN PARA LEER DHT11 --------
bool readDHT() {
  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();
  
  if (isnan(newTemp) || isnan(newHum)) {
    return false;
  }
  
  temp = newTemp;
  hum = newHum;
  
  if (temp > tempMax) tempMax = temp;
  if (temp < tempMin) tempMin = temp;
  if (hum > humMax) humMax = hum;
  if (hum < humMin) humMin = hum;
  
  return true;
}

// -------- FUNCIÓN PARA DETERMINAR MODO DÍA/NOCHE --------
bool checkDayNightMode() {
  lightLevel = readLightSensor();
  return (lightLevel <= 2800);
}

// -------- FUNCIÓN PARA ACTUALIZAR FECHA Y HORA --------
void updateDateTime() {
  DateTime now = rtc.now();
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  currentTime = String(timeStr);
  
  char dateStr[11];
  sprintf(dateStr, "%02d/%02d/%04d", now.day(), now.month(), now.year());
  currentDate = String(dateStr);
}

// -------- FUNCIÓN PARA LEER TODOS LOS SENSORES --------
void readAllSensors() {
  if (systemEnabled) {
    // Solo leer sensores si el sistema está activado
    readDHT();
    airQuality = readAirQuality();
    if (airQuality > airQualityMax) airQualityMax = airQuality;
    if (airQuality < airQualityMin) airQualityMin = airQuality;
    isDayMode = checkDayNightMode();
  } else {
    // Si el sistema está pausado, mostrar mensaje por 5 segundos
    if (!showPausedMessage) {
      showPausedMessage = true;
      pausedMessageTime = millis();
    }
  }
}

// -------- FUNCIÓN PARA ENVIAR DATOS JSON --------
void handleSensorData() {
  String json = "{";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"tempMax\":" + String(tempMax, 1) + ",";
  json += "\"tempMin\":" + String(tempMin, 1) + ",";
  json += "\"hum\":" + String(hum, 1) + ",";
  json += "\"humMax\":" + String(humMax, 1) + ",";
  json += "\"humMin\":" + String(humMin, 1) + ",";
  json += "\"airQuality\":" + String(airQuality, 1) + ",";
  json += "\"airQualityMax\":" + String(airQualityMax, 1) + ",";
  json += "\"airQualityMin\":" + String(airQualityMin, 1) + ",";
  json += "\"date\":\"" + currentDate + "\",";
  json += "\"time\":\"" + currentTime + "\",";
  json += "\"lightLevel\":" + String(lightLevel) + ",";
  json += "\"isDayMode\":" + String(isDayMode ? "true" : "false") + ",";
  json += "\"systemEnabled\":" + String(systemEnabled ? "true" : "false") + ",";
  json += "\"ventilationAlert\":" + String(ventilationAlert ? "true" : "false") + ",";
  json += "\"evacuationAlert\":" + String(evacuationAlert ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// -------- FUNCIÓN PARA CONTROL DEL SISTEMA --------
void handleControl() {
  if (server.hasArg("enable")) {
    systemEnabled = (server.arg("enable") == "1");
    showPausedMessage = false; // Resetear mensaje de pausa
    server.send(200, "text/plain", "OK");
  }
}

// -------- FUNCIÓN PARA MOSTRAR IP EN OLED --------
void showIPOnOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("IP del Sistema:");
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(WiFi.localIP());
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println("Conectate a esta IP");
  display.display();
}

// -------- FUNCIÓN PARA MOSTRAR ESTACIÓN METEOROLÓGICA EN OLED --------
void showStationOnOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.println("Estacion");
  display.setCursor(0, 30);
  display.println("Meteoro-");
  display.setCursor(0, 50);
  display.println("logica");
  display.display();
}

// -------- FUNCIÓN PARA MOSTRAR SISTEMA PAUSADO EN OLED --------
void showPausedOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Título
  display.setCursor(10, 5);
  display.println("SISTEMA PAUSADO");
  
  // Línea separadora
  display.drawLine(0, 15, 128, 15, SSD1306_WHITE);
  
  // Mensaje principal
  display.setCursor(5, 25);
  display.println("Presione el boton");
  display.setCursor(15, 35);
  display.println("en la web para");
  display.setCursor(25, 45);
  display.println("activar");
  
  display.display();
}

// -------- FUNCIÓN PARA OBTENER DESCRIPCIÓN CALIDAD AIRE --------
String getAirQualityDescription(float ppm) {
  if (ppm < 300) return "EXCELENTE";
  else if (ppm < 600) return "BUENA";
  else if (ppm < 1000) return "MODERADA";
  else if (ppm < 2000) return "MALA";
  else if (ppm < 3000) return "MUY MALA";
  else return "PELIGROSA";
}

// -------- FUNCIÓN PARA ACTUALIZAR OLED --------
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);

  // MOSTRAR PANTALLA DE ALERTA DE EVACUACIÓN (≥3000 PPM)
  if (evacuationAlert) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 0);
    display.print("PELIGRO!");
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("EVACUAR O");
    display.setCursor(0, 30);
    display.print("VENTILAR YA!");
    display.setCursor(0, 40);
    display.print("VALOR: ");
    display.print(airQuality, 0);
    display.print(" ppm");
    display.setCursor(0, 55);
    display.print("AIRE PELIGROSO");
    display.display();
    return;
  }

  // MOSTRAR PANTALLA DE ALERTA DE VENTILACIÓN (2000-2999 PPM)
  if (ventilationAlert) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.print("ALERTA!");
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("VENTILAR AREA");
    display.setCursor(0, 30);
    display.print("VALOR: ");
    display.print(airQuality, 0);
    display.print(" ppm");
    display.setCursor(0, 45);
    display.print("CALIDAD MUY MALA");
    display.setCursor(0, 55);
    display.print("ABRIR VENTANAS");
    display.display();
    return;
  }

  // Mostrar mensaje de pausa si el sistema está desactivado
  if (!systemEnabled) {
    if (showPausedMessage && (millis() - pausedMessageTime < 5000)) {
      // Mostrar mensaje por 5 segundos después de pausar
      showPausedOLED();
    } else {
      // Después de 5 segundos, mostrar datos con indicador de pausa
      showPausedMessage = false;
      
      // Encabezado con indicador de pausa
      display.setCursor(0, 0);
      display.print("[PAUSADO]");
      
      // Fecha y hora (siempre se actualizan)
      display.setCursor(70, 0);
      display.print(currentTime);
      
      // Línea separadora
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
      
      // Mostrar últimos valores registrados UNO DEBAJO DEL OTRO
      display.setCursor(0, 15);
      display.print("Temp: ");
      display.print(temp, 1);
      display.print("C");
      
      display.setCursor(0, 25);
      display.print("Hum:  ");
      display.print(hum, 1);
      display.print("%");
      
      display.setCursor(0, 35);
      display.print("Aire: ");
      display.print(airQuality, 1);
      display.print("ppm");
      
      display.setCursor(0, 45);
      display.print("Cal:  ");
      display.print(getAirQualityDescription(airQuality));
      
      // Día/Noche
      display.setCursor(0, 55);
      display.print(isDayMode ? "DIA" : "NOCHE");
    }
  } else {
    // Sistema activado - mostrar datos normales UNO DEBAJO DEL OTRO
    
    // Fecha y hora
    display.setCursor(0, 0);
    display.print(currentDate);
    display.setCursor(70, 0);
    display.print(currentTime);
    
    // Línea separadora
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Temperatura - línea 1
    display.setCursor(0, 15);
    display.print("Temp: ");
    display.print(temp, 1);
    display.print("C");
    
    // Humedad - línea 2
    display.setCursor(0, 25);
    display.print("Hum:  ");
    display.print(hum, 1);
    display.print("%");
    
    // Calidad del aire - línea 3
    display.setCursor(0, 35);
    display.print("Aire: ");
    display.print(airQuality, 1);
    display.print("ppm");
    
    // Estado de calidad del aire - línea 4
    display.setCursor(0, 45);
    display.print("Cal:  ");
    display.print(getAirQualityDescription(airQuality));
    
    // Nivel de luz y Día/Noche - línea 5
    display.setCursor(0, 55);
    display.print("Luz:  ");
    display.print(lightLevel);
    display.setCursor(70, 55);
    display.print(isDayMode ? "DIA" : "NOCHE");
  }

  display.display();
}

// -------- CONFIGURACIÓN INICIAL --------
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\n🚀 Iniciando Sistema Climático ESP32-C6...");

  // CONFIGURAR PINES
  pinMode(MQ135_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // INICIALIZAR SENSORES
  dht.begin();
  delay(2000);

  // CONECTAR WiFi
  Serial.println("📡 Conectando a WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Conectando");
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi CONECTADO");
    Serial.print("📶 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("❌ FALLO WiFi");
  }

  // INICIALIZAR I2C
  Wire.begin(I2C_SDA, I2C_SCL, 100000);
  delay(100);

  // INICIALIZAR OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED no encontrada");
  } else {
    Serial.println("✅ OLED inicializada");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // MOSTRAR IP EN OLED POR 3 SEGUNDOS
  if (WiFi.status() == WL_CONNECTED) {
    showIPOnOLED();
    delay(3000);
  }

  // MOSTRAR ESTACIÓN METEOROLÓGICA POR 3 SEGUNDOS
  showStationOnOLED();
  delay(3000);

  // INICIALIZAR RTC
  if (!rtc.begin()) {
    Serial.println("❌ RTC no encontrado");
  } else {
    Serial.println("✅ RTC inicializado");
    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }

  // CONFIGURAR SERVIDOR WEB
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });
  server.on("/sensors", HTTP_GET, handleSensorData);
  server.on("/control", HTTP_GET, handleControl);
  server.begin();
  Serial.println("🌐 Servidor web iniciado");

  // LECTURA INICIAL
  readAllSensors();
  updateDateTime();

  Serial.println("✅ SISTEMA LISTO");
}

// -------- BUCLE PRINCIPAL --------
void loop() {
  server.handleClient();
  
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 3000) {
    readAllSensors();
    updateDateTime();
    updateOLED();
    lastUpdate = millis();
  }
  
  delay(100);
}
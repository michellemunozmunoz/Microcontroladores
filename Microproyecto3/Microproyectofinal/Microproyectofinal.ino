// ================== ESTACIÓN METEOROLÓGICA IoT ESP32 ==================
// DHT11 (Temp/Humedad) + RTC (Fecha/Hora) + MQ-135 (Calidad de aire)
// + LDR para modo día/noche + OLED + App Web con botón ON/OFF
// ======================================================================

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- CONFIGURACIÓN WiFi ----------------
const char* ssid     = "Michelle";
const char* password = "1059236986";

// ---------------- SENSOR DHT11 ----------------
#define DHTPIN  4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------- SERVIDOR WEB ----------------
WebServer server(80);

// ---------------- RTC (DS3231 / DS1307) ----------------
RTC_DS3231 rtc;   // Si usas DS1307 también funciona con RTClib

// ---------------- OLED SSD1306 I2C ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1        // Sin pin de reset físico
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- MQ-135 y LDR ----------------
const int MQ135_PIN          = 34;   // Entrada analógica para MQ-135
const int LDR_PIN            = 35;   // Entrada analógica para LDR
const int LDR_DAY_THRESHOLD  = 2500; // Umbral para día/noche (ajusta según pruebas)

// ---------------- VARIABLES GLOBALES DE SENSORES ----------------
float temp    = 0.0;
float hum     = 0.0;
float tempMax = -1000.0, tempMin = 1000.0;
float humMax  = -1000.0, humMin  = 1000.0;

int airRaw    = 0;              // Lectura ADC del MQ-135
int ldrRaw    = 0;              // Lectura ADC del LDR

String airQualityStr = "Desconocida";
bool   isDayMode     = true;
bool   systemOn      = true;    // Sistema activo/inactivo (controlado desde la web)

// ---------------- CONTROL DE TIEMPOS ----------------
unsigned long lastUpdateMillis  = 0;
const unsigned long UPDATE_INTERVAL = 2000;  // 2 segundos

// ---------------- HTML DE LA APP WEB ----------------
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Estación Meteorológica IoT</title>
  <style>
    html, body {
      margin: 0;
      padding: 0;
      font-family: Arial, sans-serif;
    }
    body.day {
      background-color: #f2f2f2;
      color: #111;
    }
    body.night {
      background-color: #121212;
      color: #f2f2f2;
    }
    h2 {
      text-align: center;
      padding: 20px 10px;
    }
    .container {
      max-width: 900px;
      margin: 0 auto 30px auto;
      padding: 0 10px 20px 10px;
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 15px;
    }
    .card {
      border-radius: 10px;
      padding: 15px;
      border: 1px solid #ccc;
      box-shadow: 0 2px 6px rgba(0,0,0,0.1);
      background: rgba(255,255,255,0.9);
    }
    body.night .card {
      background: #1e1e1e;
      border-color: #333;
      box-shadow: 0 2px 6px rgba(0,0,0,0.6);
    }
    .card h3 {
      margin: 0 0 8px 0;
      font-size: 1.1rem;
    }
    .big {
      font-size: 1.8rem;
      font-weight: bold;
    }
    .small {
      font-size: 0.9rem;
    }
    .badge {
      display: inline-block;
      padding: 4px 8px;
      border-radius: 999px;
      font-size: 0.85rem;
      margin-top: 6px;
    }
    .badge.good { background: #4caf50; color: #fff; }
    .badge.mid  { background: #ff9800; color: #fff; }
    .badge.bad  { background: #f44336; color: #fff; }
    .footer {
      text-align: center;
      margin-bottom: 20px;
    }
    #toggle-btn {
      padding: 10px 20px;
      font-size: 1rem;
      border-radius: 20px;
      border: none;
      cursor: pointer;
      margin-top: 10px;
    }
    body.day #toggle-btn {
      background-color: #1976d2;
      color: #fff;
    }
    body.night #toggle-btn {
      background-color: #90caf9;
      color: #000;
    }
  </style>
</head>
<body class="day">
  <h2>ESP32 Estación Meteorológica IoT</h2>
  <div class="container">
    <div class="card">
      <h3>🌡 Temperatura</h3>
      <div class="big"><span id="temperature">--</span> °C</div>
      <div class="small">Máx: <span id="temperature-max">--</span> °C |
        Mín: <span id="temperature-min">--</span> °C</div>
    </div>

    <div class="card">
      <h3>💧 Humedad</h3>
      <div class="big"><span id="humidity">--</span> %</div>
      <div class="small">Máx: <span id="humidity-max">--</span> % |
        Mín: <span id="humidity-min">--</span> %</div>
    </div>

    <div class="card">
      <h3>📅 Fecha y hora (RTC)</h3>
      <div class="big"><span id="date">--/--/----</span></div>
      <div class="big"><span id="time">--:--:--</span></div>
    </div>

    <div class="card">
      <h3>🌫 Calidad del aire (MQ-135)</h3>
      <div class="big"><span id="air-raw">--</span></div>
      <div id="air-quality" class="badge">--</div>
    </div>

    <div class="card">
      <h3>☀️ Modo luz (LDR)</h3>
      <div class="big"><span id="mode">--</span></div>
      <div class="small">Lectura LDR: <span id="ldr">--</span></div>
    </div>

    <div class="card">
      <h3>⚙️ Estado del sistema</h3>
      <div class="big" id="system-status">--</div>
      <button id="toggle-btn" onclick="toggleSystem()">Apagar sistema</button>
    </div>
  </div>

  <div class="footer">
    Datos actualizados cada 2 segundos desde el ESP32.
  </div>

<script>
function applyMode(mode) {
  if (mode === 'day') {
    document.body.className = 'day';
  } else {
    document.body.className = 'night';
  }
}

function updateData() {
  fetch('/sensors')
    .then(response => response.json())
    .then(data => {
      document.getElementById('temperature').textContent = data.temp.toFixed(1);
      document.getElementById('temperature-max').textContent = data.tempMax.toFixed(1);
      document.getElementById('temperature-min').textContent = data.tempMin.toFixed(1);

      document.getElementById('humidity').textContent = data.hum.toFixed(1);
      document.getElementById('humidity-max').textContent = data.humMax.toFixed(1);
      document.getElementById('humidity-min').textContent = data.humMin.toFixed(1);

      document.getElementById('air-raw').textContent = data.airRaw;
      const airQuality = document.getElementById('air-quality');
      airQuality.textContent = data.airQuality;
      airQuality.className = 'badge';
      if (data.airQuality === 'Excelente' || data.airQuality === 'Buena') {
        airQuality.classList.add('good');
      } else if (data.airQuality === 'Regular') {
        airQuality.classList.add('mid');
      } else if (data.airQuality === 'Mala') {
        airQuality.classList.add('bad');
      }

      document.getElementById('date').textContent = data.date;
      document.getElementById('time').textContent = data.time;

      document.getElementById('ldr').textContent = data.ldrRaw;
      document.getElementById('mode').textContent = (data.mode === 'day') ? 'DÍA' : 'NOCHE';

      document.getElementById('system-status').textContent = data.systemOn ? 'ENCENDIDO' : 'APAGADO';
      document.getElementById('toggle-btn').textContent = data.systemOn ? 'Apagar sistema' : 'Encender sistema';

      applyMode(data.mode);
    })
    .catch(err => {
      console.log('Error al obtener datos:', err);
    });
}

function toggleSystem() {
  fetch('/toggle', { method: 'POST' })
    .then(response => response.json())
    .then(data => {
      document.getElementById('system-status').textContent = data.systemOn ? 'ENCENDIDO' : 'APAGADO';
      document.getElementById('toggle-btn').textContent = data.systemOn ? 'Apagar sistema' : 'Encender sistema';
    })
    .catch(err => {
      console.log('Error al cambiar estado:', err);
    });
}

setInterval(updateData, 2000);
window.onload = updateData;
</script>
</body>
</html>
)rawliteral";

// ---------------- FUNCIONES AUXILIARES ----------------
String twoDigits(uint8_t n) {
  if (n < 10) return "0" + String(n);
  return String(n);
}

// Lee sensores y actualiza OLED (cada UPDATE_INTERVAL ms)
void updateSensorsAndDisplay() {
  unsigned long nowMillis = millis();
  if (nowMillis - lastUpdateMillis < UPDATE_INTERVAL) return;
  lastUpdateMillis = nowMillis;

  if (!systemOn) {
    // Solo mostrar que el sistema está apagado en la OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("OFF");
    display.display();
    return;
  }

  // -------- DHT11: temperatura y humedad --------
  float newTemp = dht.readTemperature();
  float newHum  = dht.readHumidity();

  if (!isnan(newTemp) && !isnan(newHum)) {
    temp = newTemp;
    hum  = newHum;

    if (temp > tempMax) tempMax = temp;
    if (temp < tempMin) tempMin = temp;
    if (hum  > humMax)  humMax  = hum;
    if (hum  < humMin)  humMin  = hum;
  }

  // -------- MQ-135: calidad de aire --------
  airRaw = analogRead(MQ135_PIN);

  if (airRaw < 1000) {
    airQualityStr = "Excelente";
  } else if (airRaw < 2000) {
    airQualityStr = "Buena";
  } else if (airRaw < 3000) {
    airQualityStr = "Regular";
  } else {
    airQualityStr = "Mala";
  }

  // -------- LDR: modo día/noche --------
  ldrRaw = analogRead(LDR_PIN);
  isDayMode = (ldrRaw > LDR_DAY_THRESHOLD);

  // -------- RTC: fecha y hora --------
  DateTime now = rtc.now();
  String dateStr = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
  String timeStr = twoDigits(now.hour()) + ":" + twoDigits(now.minute()) + ":" + twoDigits(now.second());

  // -------- Actualizar OLED --------
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("T:");
  display.print(temp, 1);
  display.print("C  H:");
  display.print(hum, 1);
  display.println("%");

  display.setCursor(0, 10);
  display.print("Air: ");
  display.print(airQualityStr);
  display.print(" (");
  display.print(airRaw);
  display.println(")");

  display.setCursor(0, 20);
  display.print("LDR: ");
  display.print(ldrRaw);
  display.print("  ");
  display.print(isDayMode ? "DIA" : "NOCHE");

  display.setCursor(0, 30);
  display.print(dateStr);

  display.setCursor(0, 40);
  display.print(timeStr);

  display.display();
}

// Enviar datos en formato JSON para la app web
void handleSensorData() {
  // Siempre tomamos el tiempo actual del RTC para el JSON
  DateTime now = rtc.now();
  String dateStr = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
  String timeStr = twoDigits(now.hour()) + ":" + twoDigits(now.minute()) + ":" + twoDigits(now.second());

  String json = "{";
  json += "\"temp\":"      + String(temp, 1) + ",";
  json += "\"tempMax\":"   + String(tempMax, 1) + ",";
  json += "\"tempMin\":"   + String(tempMin, 1) + ",";
  json += "\"hum\":"       + String(hum, 1) + ",";
  json += "\"humMax\":"    + String(humMax, 1) + ",";
  json += "\"humMin\":"    + String(humMin, 1) + ",";
  json += "\"airRaw\":"    + String(airRaw) + ",";
  json += "\"airQuality\":\"" + airQualityStr + "\",";
  json += "\"ldrRaw\":"    + String(ldrRaw) + ",";
  json += "\"mode\":\""    + String(isDayMode ? "day" : "night") + "\",";
  json += "\"systemOn\":"  + String(systemOn ? "true" : "false") + ",";
  json += "\"date\":\""    + dateStr + "\",";
  json += "\"time\":\""    + timeStr + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// Cambiar ON/OFF del sistema desde el botón web
void handleToggle() {
  systemOn = !systemOn;

  String json = "{";
  json += "\"systemOn\":" + String(systemOn ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// ---------------- SETUP ----------------
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\nIniciando ESP32 Estacion...");

  // DHT11
  dht.begin();

  // MQ-135 y LDR
  pinMode(MQ135_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // I2C, RTC y OLED
  Wire.begin();   // SDA=21, SCL=22 en la mayoría de ESP32

  if (!rtc.begin()) {
    Serial.println("⚠️ No se encuentra RTC");
  } else {
    if (rtc.lostPower()) {
      // Ajustar RTC con la fecha/hora de compilación
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      Serial.println("RTC ajustado con fecha/hora de compilacion.");
    }
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("⚠️ No se encuentra OLED");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Estacion Meteorologica");
    display.setCursor(0, 10);
    display.println("Iniciando...");
    display.display();
  }

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi");
  Serial.print("📶 IP asignada: ");
  Serial.println(WiFi.localIP());

  // Rutas del servidor web
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });
  server.on("/sensors", HTTP_GET, handleSensorData);
  server.on("/toggle", HTTP_POST, handleToggle);

  server.begin();
  Serial.println("🌐 Servidor web iniciado");

  // Primera actualización de sensores y OLED
  updateSensorsAndDisplay();
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
  updateSensorsAndDisplay();
}

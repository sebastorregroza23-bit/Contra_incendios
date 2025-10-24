/*
  Sistema Antiincendios con ESP32 + MQ-02 + Sensor Infrarrojo + Buzzer + Telegram
  
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Internet
const char* WIFI_SSID = "nombre de la red";
const char* WIFI_PASS = "contraseña de tu red";

const String BOT_TOKEN = "tu id token de telegram";
const String CHAT_ID   = "tu chad id telegram"; // tu chat_id personal

// pines
#define MQ2_PIN        34   // Entrada analógica del sensor MQ-02
#define IR_SENSOR_PIN  15   // Sensor infrarrojo digital
#define BUZZER_PIN      4   // Buzzer activo con HIGH

// UMBRALES 
#define MQ2_UMBRAL     3800  // Valor analógico aproximado de detección de humo (ajustable)

// VARIABLES
WiFiClientSecure client;
const char* host = "api.telegram.org";
long updateOffset = 0;
unsigned long lastUpdatePoll = 0;
const unsigned long UPDATE_INTERVAL = 4000; // ms entre revisiones

bool alertaActiva = false;    // Evita mensajes repetidos
bool humoDetectado = false;
bool fuegoDetectado = false;

// FUNCIONES
void sendTelegram(String message) {
  if (WiFi.status() != WL_CONNECTED) return;

  String path = "/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + urlencode(message);

  if (!client.connect(host, 443)) {
    Serial.println("Fallo conexión con Telegram");
    return;
  }

  client.printf("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                path.c_str(), host);

  while (client.connected() || client.available()) client.read();
  client.stop();
  Serial.println("Mensaje enviado a Telegram: " + message);
}

String urlencode(const String &str) {
  String encoded = "";
  char c, buf[4];
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') encoded += c;
    else { sprintf(buf, "%%%02X", (unsigned char)c); encoded += buf; }
  }
  return encoded;
}

void handleTelegramMessages() {
  if (WiFi.status() != WL_CONNECTED) return;

  String path = "/bot" + BOT_TOKEN + "/getUpdates?timeout=0";
  if (updateOffset) path += "&offset=" + String(updateOffset);

  if (!client.connect(host, 443)) {
    Serial.println("No se pudo conectar a Telegram");
    return;
  }

  client.printf("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                path.c_str(), host);

  String response = "", line;
  bool headersEnded = false;
  while (client.connected() || client.available()) {
    line = client.readStringUntil('\n');
    if (!headersEnded && line == "\r") headersEnded = true;
    else if (headersEnded) response += line;
  }
  client.stop();

  if (response.length() == 0) return;

  DynamicJsonDocument doc(15 * 1024);
  if (deserializeJson(doc, response)) return;

  JsonArray results = doc["result"].as<JsonArray>();
  for (JsonObject update : results) {
    long updateId = update["update_id"].as<long>();
    updateOffset = updateId + 1;

    if (!update.containsKey("message")) continue;

    String text = update["message"]["text"].as<String>();
    String fromChat = String(update["message"]["chat"]["id"].as<long>());

    if (fromChat != CHAT_ID) continue;

    Serial.println("Mensaje recibido desde Telegram: " + text);

    if (text.equalsIgnoreCase("/start")) {
      sendTelegram("🔥 Sistema antiincendios activado. Monitoreando humo y fuego.");
    } else if (text.equalsIgnoreCase("/estado")) {
      String estado = "📊 Estado actual:\n";
      estado += humoDetectado ? "💨 Humo: DETECTADO\n" : "💨 Humo: Normal\n";
      estado += fuegoDetectado ? "🔥 Fuego: DETECTADO\n" : "🔥 Fuego: Normal\n";
      sendTelegram(estado);
    } else {
      sendTelegram("📩 Comando no reconocido. Usa /start o /estado.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando sistema antiincendios...");

  pinMode(MQ2_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  client.setInsecure();

  sendTelegram("🤖 ESP32 conectado. Sistema de detección de humo y fuego listo.");
}

void loop() {
  unsigned long now = millis();

  // === LECTURA DE SENSORES ===
  int mq2Value = analogRead(MQ2_PIN);
  bool irValue = digitalRead(IR_SENSOR_PIN);

  humoDetectado = mq2Value > MQ2_UMBRAL;
  fuegoDetectado = irValue == LOW; // depende del módulo IR (HIGH = detecta fuego/movimiento)

  Serial.print("MQ2: "); Serial.print(mq2Value);
  Serial.print(" | IR: "); Serial.println(irValue);

  // === LÓGICA DE ALERTAS ===
  if ((humoDetectado || fuegoDetectado) && !alertaActiva) {
    alertaActiva = true;
    digitalWrite(BUZZER_PIN, HIGH);

    if (humoDetectado && fuegoDetectado) {
      sendTelegram("🚨 ALERTA CRÍTICA: Se detectó HUMO y FUEGO simultáneamente ⚠");
    } else if (humoDetectado) {
      sendTelegram("💨 ALERTA: Se detectó humo o gas con el sensor MQ-02.");
    } else if (fuegoDetectado) {
      sendTelegram("🔥 ALERTA: Sensor infrarrojo detecta fuego o movimiento.");
    }
  } 
  else if (!humoDetectado && !fuegoDetectado && alertaActiva) {
    alertaActiva = false;
    digitalWrite(BUZZER_PIN, LOW);
    sendTelegram("✅ Zona segura. No se detecta humo ni fuego.");
  }

  // === RECEPCIÓN DE MENSAJES DE TELEGRAM ===
  if (now - lastUpdatePoll >= UPDATE_INTERVAL) {
    lastUpdatePoll = now;
    handleTelegramMessages();
  }

  delay(300);
}

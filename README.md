# 🔥 Detector de Incendios con ESP32 y Telegram

Este proyecto implementa un **sistema de alarma contra incendios** con **ESP32**, capaz de detectar humo y fuego en tiempo real.  
Cuando se activa una condición de riesgo, el sistema enciende un **buzzer** y envía **alertas a Telegram** mediante un bot, siguiendo protocolos básicos de prevención de incendios.

---

## 📌 Objetivo
Proporcionar una solución **económica, accesible y de bajo costo** para la **detección temprana de incendios**, que pueda integrarse en hogares, colegios o negocios, reforzando la seguridad y facilitando la **evacuación rápida**.

---

## ⚙️ Características principales
- 🔎 **Detección de humo y gases** con el sensor **MQ-2**.  
- 🔥 **Detección de fuego** mediante sensor infrarrojo de flama.  
- 🔔 **Alerta sonora** con buzzer.  
- 📲 **Notificación automática en Telegram** a través de un bot.  
- 🛠️ **Comandos disponibles en Telegram**:
  - `/start` → Inicia y confirma que el sistema está activo.
  - `/estado` → Consulta el estado actual (humo / fuego).
- ✅ Restablecimiento automático: notificación cuando la zona vuelve a estar segura.  

---

## 🛠️ Materiales
- ESP32 (placa de desarrollo)  
- Sensor de humo MQ-2  
- Sensor de flama infrarrojo (IR)  
- Buzzer activo  
- Protoboard y cables Dupont  

---

## 📡 Conexión de pines
- **MQ-2** → Entrada analógica GPIO34  
- **IR Flama** → Entrada digital GPIO15  
- **Buzzer** → Salida digital GPIO4  

---

## 📲 Configuración de Telegram
1. Crear un bot con [BotFather](https://core.telegram.org/bots#botfather).  
2. Copiar el **TOKEN** que te entrega BotFather.  
3. Obtener tu **Chat ID** con el bot `@userinfobot`.  
4. Configurar en el código:

```cpp
const char* WIFI_SSID = "TU_WIFI";
const char* WIFI_PASS = "TU_PASSWORD";

const String BOT_TOKEN = "TU_TOKEN";
const String CHAT_ID   = "TU_CHAT_ID";

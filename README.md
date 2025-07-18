# SmartRoom-Controller

An IoT-based smart room system built using the **ESP32** microcontroller. This project enables real-time monitoring and control of room **temperature** and **humidity** via a **mobile dashboard** using **MQTT protocol**. It supports both **manual** and **automatic fan control** based on a user-defined temperature threshold.

---

## 🚀 Features

- 🌡️ Real-time Temperature & Humidity Monitoring (DHT11 sensor)
- 🌬️ Cooling Fan Control (Auto & Manual Modes)
- 📲 Mobile Dashboard Integration (MQTT-compatible apps like MQTT Dash or IoT MQTT Panel)
- 🔁 Automatic operation based on a configurable temperature limit
- 📡 Communication via Wi-Fi and MQTT (using public broker)

---

## 🧰 Hardware Used

- ESP32 Dev Module (30-pin)
- DHT11 Temperature & Humidity Sensor
- DC Fan
- L289 H-Bridge Driver Module
- Power supply for fan
- Flyback diode
- Resistors and breadboard

---

## 🧪 Software & Libraries

- [Arduino IDE](https://www.arduino.cc/en/software)
- Libraries used:
  - `WiFi.h`
  - `PubSubClient.h`
  - `DHT.h`

---

## 🗂️ Project Structure

```plaintext
SmartRoom-Controller/
├── Project.ino             # Main Arduino code
├── README.md               # This file

# ESP32 based Firebase Temperature Logger with Relay Control

This project demonstrates how to use an ESP32 to log temperature data to Firebase Realtime Database and control a relay based on a timed interval or based on the Temperature. So, it can use to control external device to turn on or turn off using relays. The temperature is read from a DS18B20 sensor, and the relay is toggled on and off periodically. The temperature data and relay state are uploaded to Firebase, making them accessible remotely.

This device is used to log the temperature of the experimental setup and control the external power source using relay.

## Features

- Connects to a WiFi network.
- Synchronizes time using NTP.
- Reads temperature data from a DS18B20 sensor.
- Logs temperature and relay state to Firebase Realtime Database.
- Implements retry mechanisms for network and Firebase operations.
- Controls a relay with a customizable on/off cycle.

## Hardware Requirements

- ESP32 DevKit V1
- DS18B20 Temperature Sensor
- Relay Module
- Breadboard and jumper wires
- 4.7k Ohm resistor (for the DS18B20 sensor)
- Power supply (5V for ESP32)

## Libraries Used

- [WiFi](https://github.com/espressif/arduino-esp32)
- [Firebase ESP Client](https://github.com/mobizt/Firebase-ESP-Client)
- [OneWire](https://github.com/PaulStoffregen/OneWire)
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)

## Setup Instructions

### 1. Install Required Libraries

Make sure you have the following libraries installed in your Arduino IDE:

- WiFi
- Firebase ESP Client
- OneWire
- DallasTemperature

### 2. Wiring Diagram

Connect your hardware as follows:

| ESP32 Pin | DS18B20 Pin | Relay Module Pin |
| --------- | ----------- | ---------------- |
| GPIO 4    | Data        | N/A              |
| 3.3V      | VCC         | VCC              |
| GND       | GND         | GND              |
| GPIO 14   | N/A         | IN1              |
| GPIO 18   | N/A         | IN2              |

### 3. Firebase Setup

1. Create a Firebase project in the Firebase console.
2. Set up a Realtime Database and note the database URL.
3. Obtain your API key from the project settings.

### 4. Code Configuration

Replace the placeholders in the code with your WiFi credentials and Firebase project details:

```cpp
#define WIFI_SSID "your-ssid"
#define WIFI_PASSWORD "your-password"
#define API_KEY "your-api-key"
#define DATABASE_URL "your-database-url"
```

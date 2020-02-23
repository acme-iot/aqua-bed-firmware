/*
 * Aquaponic planter firmware
 */


#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif


extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

#include <AsyncMqttClient.h>


#define WIFI_SSID "yourSSID"
#define WIFI_PASSWORD "yourpass"

#define MQTT_HOST IPAddress(192, 168, 1, 10)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing...");

}

void loop() {
  delay(2000);
  Serial.print(".");
}
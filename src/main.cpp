/*
 * Aquaponic planter firmware
 */


#ifdef ESP32
#include <WiFi.h>
#include <WiFiType.h>

#include <Arduino.h>
//#include <Hash.h>
#include <FS.h>

//#include "wifi.h"
#include <conf.h>
#include <mqtt.h>
//#include <jsoninfo.h>
#include <webapi.h>
#include <SPIFFSAccess.h>


#else
#include <ESP8266WiFi.h>
#endif

#ifndef CI_BUILD
#include "secrets.h"
#else
#define SECRET_SSID "SSID"
#define SECRET_PASSWORD "password1234"
#define SECRET_MQTT_USERNAME ""
#define SECRET_MQTT_PASSWORD ""
#endif


extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

#include <ArduinoLog.h>
#include <AsyncMqttClient.h>


#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#endif


Config *g_cfg;
SPIFFSAccess *g_file;

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;
const char* mqtt_username = SECRET_MQTT_USERNAME;
const char* mqtt_password = SECRET_MQTT_PASSWORD;

#define MQTT_PORT 1883
#define MQTT_HOST IPAddress(54, 70, 96, 251)

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;


void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        //Serial.println(g_cfg->getConfig());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		    xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("test/lol", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.print("  payload: ");
  Serial.println(payload);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setupDebugging() {
  Serial.setDebugOutput(true);
  //esp_log_level_set("*", ESP_LOG_VERBOSE);

}

void setupLogging() {
  Serial.begin(115200);
  while(!Serial && !Serial.available()){}
  Log.begin(LOG_LEVEL, &Serial);
}

void setupFileSystem() {
  g_file->begin();
}


void setup() {
  setupDebugging();
  setupLogging();
  setupFileSystem();

  Log.notice("Running...");

  pinMode(LED_BUILTIN, OUTPUT);

  Log.trace("MAC: %x", WiFi.macAddress());

  g_cfg->loadConfig();

  return;

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(mqtt_username, mqtt_password);

  connectToWifi();
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}
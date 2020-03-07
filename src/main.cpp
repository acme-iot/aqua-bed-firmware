/*
 * Aquaponic planter firmware
 */
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
//#include <sstream> // stringstream
//#include <iomanip> // put_time
#include <string>  // string
//#include <time.h>

#include <cassert>

#include <TimeLib.h>

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
// remove since 8266 wont be supported
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

extern "C"
{
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

const int BaudRate = 115200;

Config *g_cfg;
SPIFFSAccess *g_file;

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASSWORD;
const char *mqtt_username = SECRET_MQTT_USERNAME;
const char *mqtt_password = SECRET_MQTT_PASSWORD;

#define MQTT_PORT 1883
#define MQTT_HOST IPAddress(54, 70, 96, 251)

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
TimerHandle_t timeTimer;

void connectToWifi()
{
  Log.trace("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  Log.trace("IP address: %s", WiFi.localIP());
}

void connectToMqtt()
{
  Log.trace("Connecting to MQTT...");
  mqttClient.connect();
}

void syncTimeCallback()
{
  Log.trace("Connecting to NTP server...");
  return;
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
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

void onMqttConnect(bool sessionPresent)
{
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

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected())
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
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

void onMqttPublish(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void preInitialize()
{
  // assert all that is required
  assert(Serial);

  // init Serial
  Serial.begin(BaudRate);
  while (!Serial && !Serial.available())
  {
  }
}

void setupDebugging()
{
  Serial.setDebugOutput(true);
}

void setupLogging()
{
  auto getFormattedTime = [](Print *p) {
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::high_resolution_clock::to_time_t(now);
    auto localTime = std::localtime(&time);
    auto str = std::string(asctime(localTime));
    str.pop_back();
    str += " ";
    p->print(str.c_str());
  };

  Log.begin(LOG_LEVEL, &Serial);
  Log.setPrefix(getFormattedTime);
  Log.setSuffix([](Print *p) { p->print("\n"); });
}

void setupFileSystem()
{
  g_file->begin();
}

void setup()
{
  preInitialize();
  setupDebugging();
  setupLogging();
  setupFileSystem();

  Log.notice("Running...");

  for (;;)
  {
    Log.trace("In Loop");
    delay(1000);
    break;
  }

  timeTimer = xTimerCreate("timeTimer", pdMS_TO_TICKS(5 * 1000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(syncTimeCallback));

  if (xTimerStart(timeTimer, 0) != pdPASS)
  {
    Log.error("Time timer failed.");
  }

  pinMode(LED_BUILTIN, OUTPUT);

  Log.trace("MAC: %x", WiFi.macAddress());

  g_cfg->loadConfig();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

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

void loop()
{
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}
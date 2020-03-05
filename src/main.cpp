/*
 * Aquaponic planter firmware
 */
#include <ctime>
#include <chrono>

#include <TimeLib.h>

#include <Ethernet.h>
#include <EthernetUdp.h>


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


extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

// begin NTP
#include <WiFiUdp.h>

unsigned long sendNTPpacket(IPAddress& address);

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

// end NTP

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
TimerHandle_t timeTimer;


void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  //NTP
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  //Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  //NTP
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void syncTimeCallback() {
  Log.trace("Connecting to NTP server...");
  return ;
  
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available  
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
  }
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

  auto getFormattedTime = []() {
    auto currentTimePoint = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t( currentTimePoint );
    return ctime( &currentTime );
  };

  for(;;){
    delay(1000);
    Log.trace("Time: %s", getFormattedTime());
    break;
  }

  timeTimer = xTimerCreate("timeTimer", pdMS_TO_TICKS(5 * 1000), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(syncTimeCallback));

  if(xTimerStart(timeTimer, 0) != pdPASS) {
    Log.error("Time timer failed.");
  }

  pinMode(LED_BUILTIN, OUTPUT);

  Log.trace("MAC: %x", WiFi.macAddress());

  g_cfg->loadConfig();

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
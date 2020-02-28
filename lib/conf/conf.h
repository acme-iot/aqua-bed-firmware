#ifndef ESP_LIB_CONFIG_H
#define ESP_LIB_CONFIG_H

#include "spiffs.h"
#include "ArduinoJson.h"

#define config_filename "/spiffs/config.json"
#define config_default_json "{\"unit\":{\"name\":\"SHTech-R\"},\"web\":{\"user\":\"admin\",\"pass\":\"admin\"},\"wifi\":{},\"plugins\":[]}"



class Config
{
    private:
        //StaticJsonBuffer<20000> jsonBuffer;
        StaticJsonDocument<20000> jsonBuffer;
        JsonObject /* * */ configuration;

    public:
        Config() {
            loadConfig();
        }
        void loadConfig() {
             ESP_LOGI("CONF", "loading config object");
            char* confData = read_file((char*)config_filename);
            if (confData == NULL) {
                ESP_LOGI("CONF", "warning: config file not found, creating new!");
                write_file((char*)config_filename, (char*)config_default_json, strlen(config_default_json));
                confData = read_file((char*)config_filename);
            }
            ESP_LOGI("CONF", "Parsing JSON object");
            //JsonObject& ref = jsonBuffer.parseObject(confData);
            deserializeJson(jsonBuffer, confData);
            //configuration = &ref;
        }
        JsonObject& getConfig() {
            return /***/configuration;
        }
        const char* getUnitName() {
            return (/***/configuration)["unit"]["name"].as<char*>();
        }

};

#endif
/*
#ifndef Confo_h
#define Confo_h

#include "Arduino.h"
#include "FS.h"
#include <ArduinoJson.h>
#include <WiFiClient.h>

class Configuration
{
  public:
    char hostname[16];
    // STA Configuration
    char sta_ssid[32];
    char sta_password[32];
    bool sta_dhcp = false;
    //  STA network
    char sta_ip[16];
    char sta_netmask[16];
    char sta_gateway[16];
    char sta_dns0[16];
    char sta_dns1[16];
    // AP Configuration
    char ap_ssid[32];
    char ap_password[32];
    // AP network
    char ap_ip[16];
    char ap_netmask[16];
    // MQTT Configuration
    char mqtt_server[16];
    bool mqtt_enable = false;
    int mqtt_port;
    char mqtt_user[32];
    char mqtt_password[32];
    
    Configuration();
    bool saveConfig();
    bool loadConfig();
};


#endif */
#ifndef ESP_LIB_SPIFFS_H
#define ESP_LIB_SPIFFS_H

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ArduinoJson.h"

/* Function to initialize SPIFFS */
esp_err_t spiffs_init();
esp_err_t sdcard_init(JsonObject& spi);
char* read_file(char * filename, long *len);
char* read_file(char * filename);
esp_err_t write_file(char *filepath, char * data, uint16_t length);

// static esp_err_t spiffs_delete_file() {

// }

// static esp_err_t spiffs_copy_file() {

// }

// static esp_err_t spiffs_rename_file() {

// }


#endif
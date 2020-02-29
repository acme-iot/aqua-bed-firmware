#include "spiffs.h"
#define TAG "SPIFFS"
/* Function to initialize SPIFFS */
esp_err_t spiffs_init(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 50,   // This decides the maximum number of files that can be created on the storage
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    } 

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}

#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13

esp_err_t sdcard_init(JsonObject& spiConfig) {
    // if (!spi) {
    //     ESP_LOGI(TAG, "Initializing SD card using SDMMC peripheral");
    //     sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    //     // This initializes the slot without card detect (CD) and write protect (WP) signals.
    //     // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    //     sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    //     // To use 1-line SD mode, uncomment the following line:
    //     // slot_config.width = 1;

    //     // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    //     // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    //     // does make a difference some boards, so we do that here.
    //     gpio_set_pull_mode((gpio_num_t)15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    //     gpio_set_pull_mode((gpio_num_t)2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    //     gpio_set_pull_mode((gpio_num_t)4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    //     gpio_set_pull_mode((gpio_num_t)12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    //     gpio_set_pull_mode((gpio_num_t)13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes
    // } else {
        ESP_LOGI(TAG, "Initializing SD card using SPI peripheral");

        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
        slot_config.gpio_miso = (gpio_num_t)spiConfig["miso"].as<uint8_t>();
        slot_config.gpio_mosi = (gpio_num_t)spiConfig["mosi"].as<uint8_t>();
        slot_config.gpio_sck  = (gpio_num_t)spiConfig["sclk"].as<uint8_t>();
        slot_config.gpio_cs   = (gpio_num_t)spiConfig["cs"].as<uint8_t>();
    // }
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize the card (%s). "
            "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

esp_err_t sdcard_unmount(void) {
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(TAG, "Card unmounted");
    return ESP_OK;
}

char* read_file(char * filename, long *len) {
    ESP_LOGI(TAG, "loading file %s", filename);
    FILE *f = fopen(filename, "rb");
    if (f == NULL) return NULL;

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    ESP_LOGI(TAG, "total size of file: %lu", *len);
    char *data = (char*)malloc(*len + 1);
    fread(data, *len, 1, f);
    fclose(f);
    ESP_LOGI(TAG, "File read succesfully");
    return data;
}

char* read_file(char * filename) {
    long len;
    return read_file(filename, &len);
}

esp_err_t write_file(char *filepath, char * data, uint16_t length) {
    ESP_LOGI(TAG, "Writing file %s with  bytes", filepath);
    FILE *fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        return ESP_FAIL;
    }
    fwrite(data, 1, length, fd);
    fclose(fd);
    ESP_LOGI(TAG, "File write succesfull");
    return ESP_OK;
}

// static esp_err_t spiffs_delete_file() {

// }

// static esp_err_t spiffs_copy_file() {

// }

// static esp_err_t spiffs_rename_file() {

// }

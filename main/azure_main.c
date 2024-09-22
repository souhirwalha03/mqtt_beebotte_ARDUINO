#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/i2c.h"
#include "mbedtls/ssl.h"


#include "esp_system.h"
#include "esp_system.h"
#include "esp_wifi.h"
#ifdef CONFIG_IDF_TARGET_ESP8266 || (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0))
#include "esp_event_loop.h"
#else
#include "esp_event.h"
#endif
#include "esp_log.h"
#include "esp_tls.h"
#include "nvs_flash.h"
//#include <portmacro.h>

#include "rc522.h" // C library for interfacing ESP32 with MFRC522 RFID card reader.

#include "beebotte.h"
#include "iothub_rfid.h"

#define EXAMPLE_WIFI_SSID "Wokwi-GUEST"
#define EXAMPLE_WIFI_PASS ""

EventGroupHandle_t wifi_event_group;

rc522_event_data_t event_data;
rc522_tag_t fake_tag;
#ifndef BIT0
#define BIT0 (0x1 << 0)
#endif
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static const char *TAG = "azure";
static rc522_handle_t scanner;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        /* This is a workaround as ESP platform WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
}




static void initialise_wifi(void)
{

    ESP_ERROR_CHECK( esp_netif_init() );
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);

#ifdef CONFIG_IDF_TARGET_ESP8266 || (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0))
#else
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
#endif

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}



//send RFID tag ID
static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    printf("******** EXECUTING rc522_handler ********\r\n");
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;

    switch(event_id) {
        // triggered when a tag is scanned by the RFID reader
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                ESP_LOGI(TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
                iothub_rfid(tag->serial_number);
            //xTaskCreate(iothub_rfid, "iot_hub_task", 4096, (void *)&tag->serial_number, 5, NULL);


            }
            break;
    }

    
}



void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    initialise_wifi();
    
    rc522_config_t config = {
        .spi.host = VSPI_HOST,
        .spi.miso_gpio = 25,
        .spi.mosi_gpio = 23,
        .spi.sck_gpio = 19,
        .spi.sda_gpio = 22,
    };

    // rc522_create(&config, &scanner);

    // fake_tag.serial_number = 30; 
    // event_data.ptr = &fake_tag;
    // rc522_handler(NULL, NULL, RC522_EVENT_TAG_SCANNED, &event_data);
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    rc522_start(scanner);

    xTaskCreate(beebotte, "listen_task", 4096, NULL, 6, NULL);
    

}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/i2c.h"
#include "mbedtls/ssl.h"


#include "esp_system.h"
#include "esp_system.h"
#ifdef CONFIG_IDF_TARGET_ESP8266 || (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0))
#include "esp_event_loop.h"
#else
#include "esp_event.h"
#endif
#include "esp_log.h"
#include "esp_tls.h"
#include "nvs_flash.h"
//#include <portmacro.h>

#include "rc522.h" 

#include "beebotte.h"
#include "iothub_rfid.h"

rc522_event_data_t event_data;
rc522_tag_t fake_tag;

static const char *TAG = "azure";
static rc522_handle_t scanner;


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
    printf("1\n");
    
    rc522_config_t config = {
        .spi.host = VSPI_HOST,
        .spi.miso_gpio = 25,
        .spi.mosi_gpio = 23,
        .spi.sck_gpio = 19,
        .spi.sda_gpio = 22,
    };
    rc522_create(&config, &scanner);
    printf("2\n"); 
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    //xTaskCreate(beebotte, "listen_task", 4096, NULL, 6, NULL);
    rc522_start(scanner);
    printf("3\n");


    // fake_tag.serial_number = 30; 
    // event_data.ptr = &fake_tag;
    // rc522_handler(NULL, NULL, RC522_EVENT_TAG_SCANNED, &event_data);

    

}

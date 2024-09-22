#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "PubSubClient.h"
#include "esp_system.h"
#include <WiFi.h>
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <ArduinoJson.h>
#include <SPI.h>


// const char* ssid = "Wokwi-GUEST";
// const char* password = "";


const char* ssid = "Orange";
const char* password = "20400424";

WiFiClient client;

const char* channel = "pfe";
const char* topic1 = "pfe/rfid";
const char* topic2 = "pfe/balance";
unsigned char serNum[5];

const char* server = "mqtt.beebotte.com";
const char* username = "token:token_r93FvLAUmIbGeplA";

char ClientID[20] = "";
PubSubClient clientmqtt(server,1883,client);




static const char *TAG = "beebotte";
int deserialize( byte* payload, unsigned int length){

    char message[length + 1];
        for (int i = 0; i < length; i++)
            message[i] = (char)payload[i];
        message[length] = '\0';
        Serial.print("Received Json: ");
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, message);
        Serial.println(message);
        int data = doc["data"];
        Serial.print("Data value: ");
        Serial.println(data);
        return data;
}
void callback(char* topic, byte* payload, unsigned int length)
{
    
    Serial.println( "MQTT callback executed");
    if(strcmp(topic, topic1) == 0){
        Serial.print("RFID Checked");
        int data = deserialize(payload, length);
         if (data == 1){
            Serial.print("client and balance checked\n");
            Serial.print("Start Charging\n");
            vTaskDelay(pdMS_TO_TICKS(1000));

        }else {
            Serial.print("not a client or insufficient balance\n");
             vTaskDelay(pdMS_TO_TICKS(1000));

        }

    }else if (strcmp(topic, topic2) == 0){

        int data = deserialize(payload, length);
        if (data == 1){
            Serial.print("Start Charging\n");
            vTaskDelay(pdMS_TO_TICKS(1000));

        }else {
            Serial.print("Stop Charging\n");
            vTaskDelay(pdMS_TO_TICKS(1000));

        }
    }
    

}


void connectToWiFi() {
    WiFi.begin(ssid, password);
        printf("45");

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
            printf("46");

        delay(500);
        Serial.print(".");
    }
        printf("44");

    Serial.println("Connected to WiFi");
}

void connectToMQTT() {
    printf("41");
    if (!clientmqtt.connected()) {
    printf("42");

        ESP_LOGI(TAG, " not connected");
        if (clientmqtt.connect(ClientID, username, NULL)) {
            printf("43");

            ESP_LOGI(TAG, " Connected");
            clientmqtt.subscribe(topic1);
            clientmqtt.subscribe(topic2);

        }
    }
}

 
 

extern "C" void beebotte(void)
{   printf("48");
     esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    printf("49");
    Serial.begin(115200);
    connectToWiFi();
    delay(1000);

    clientmqtt.setServer(server, 1883);
    clientmqtt.setCallback(callback);
    while (1){
        if (!clientmqtt.connected()) {
            connectToMQTT();
            printf("51");

        } else {
            //* Listening for messages...*/
            printf("52");
            clientmqtt.loop();
            vTaskDelay(pdMS_TO_TICKS(1000));

        }
    }

    
}
        
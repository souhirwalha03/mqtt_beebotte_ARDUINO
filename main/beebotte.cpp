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




static const char *TAG = "azure";
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
            Serial.print("client and balance checked");
            Serial.print("Start Charging");
        }else {
            Serial.print("not a client or insufficient balance");
        }

    }else if (strcmp(topic, topic2) == 0){
        Serial.print(" Charging");

        int data = deserialize(payload, length);
        if (data == 1){
            Serial.print("Start Charging");
        }else {
            Serial.print("Stop Charging");
        }
    }
    

}


void connectToMQTT() {
    if (!clientmqtt.connected()) {
        ESP_LOGI(TAG, "MQTT not connected");
        if (clientmqtt.connect(ClientID, username, NULL)) {
            ESP_LOGI(TAG, "MQTT Connected");
            clientmqtt.subscribe(topic1);
            clientmqtt.subscribe(topic2);
            

        }
    }
}


extern "C" void beebotte(void)
{   
    clientmqtt.setServer(server, 1883);
    clientmqtt.setCallback(callback);
    
    while (1) {
        if (!clientmqtt.connected()) {
                    connectToMQTT();
        ESP_LOGI(TAG, "connecting...");


        } else {
            clientmqtt.loop();
            ESP_LOGI(TAG, "Listening for messages...");
            vTaskDelay(pdMS_TO_TICKS(1000));

        }


    }
}
        
 

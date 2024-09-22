#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iothubtransportmqtt.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <iothub_client_ll.h>
#include <iothub_device_client_ll.h>
#include "azure_c_shared_utility/platform.h"
#include "beebotte.h"


#define RFID_TAG_LEN 12 // Adjust based on your RFID tag format
static char msgText[256];
static int callbackCounter;
#define CONFIG_MESSAGE_INTERVAL_TIME 1000

//static const char* CONNECTION_STRING = "HostName=evcshub.azure-devices.net;DeviceId=evcs;SharedAccessKey=2oCiRutRFIH6Y36/HmdtkYe8TP420psWHAIoTDE8RB0=";
//HostName=evcsHub.azure-devices.net;DeviceId=evcsDevice;SharedAccessKey=CGPPVhTlK0ml/EjrbfXDO1CZR2X021e1NAIoTL5VFfQ=
//static const char* CONNECTION_STRING = "HostName=EVSE.azure-devices.net;DeviceId=dev;SharedAccessKey=LHEMvw5t3/OmYLl2RVuaeaUh+gXydxrX6AIoTCsMI2I=";

static const char* CONNECTION_STRING = "HostName=evcsHub.azure-devices.net;DeviceId=evcsDevice;SharedAccessKey=CGPPVhTlK0ml/EjrbfXDO1CZR2X021e1NAIoTL5VFfQ=";
static bool g_continueRunning;

// static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback) {
//     printf("12\n");
//     if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
//         printf("Message sent successfully.\n");
//         callbackCounter++;
        
//     }
// }
static void send_message_to_iot_hub(IOTHUB_CLIENT_LL_HANDLE iotHubClient, const char* msgText) {
    printf("15\n");

  // Create message with RFID tag data
  
  IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromString(msgText);
    if (messageHandle == NULL) {
        printf("Failed to create message.\n");
        IoTHubClient_LL_Destroy(iotHubClient);
        platform_deinit();
        return;
    }
    printf("16\n");
    if (IoTHubClient_LL_SendEventAsync(iotHubClient, messageHandle, NULL, NULL) != IOTHUB_CLIENT_OK) {
        printf("Failed to send message.\n");
        IoTHubMessage_Destroy(messageHandle);
        IoTHubClient_LL_Destroy(iotHubClient);
        platform_deinit();
        return;
    }
    printf("17\n");
    while (g_continueRunning) {
        IoTHubClient_LL_DoWork(iotHubClient);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_MESSAGE_INTERVAL_TIME));
    }
    g_continueRunning = true;


    IoTHubMessage_Destroy(messageHandle);
      printf("18\n");
}


void iothub_rfid(uint64_t serial_number) {
        printf("11\n");

    sprintf(msgText, "{ \"deviceId\" : \"RFID\" ,\"RFID_tag\" : %llu }", serial_number);
    if (platform_init() != 0) {
        printf("Failed to initialize the platform.\n");
        return;
    }
    IOTHUB_CLIENT_LL_HANDLE iotHubClient;
    

    // Set up IoT Hub client
//    if ((iotHubClient = IoTHubClient_LL_CreateFromConnectionString(CONNECTION_STRING, MQTT_Protocol)) == NULL) {

    if ((iotHubClient = IoTHubClient_LL_CreateFromConnectionString(CONNECTION_STRING, MQTT_Protocol)) == NULL) {

        printf("Failed to create IoT Hub client");
        platform_deinit();
        return;
    }

    callbackCounter = 0;
    g_continueRunning = true;
    printf("12\n");
   printf("13\n");

        send_message_to_iot_hub(iotHubClient, msgText);
    
    printf("14\n");

    // Cleanup on exit
    IoTHubClient_LL_Destroy(iotHubClient);
    //xTaskCreate(iot_hub_task, "iot_hub_task", 4096, (void *)&msgText, 5, NULL);
}


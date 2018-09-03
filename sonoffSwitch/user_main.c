#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "debug.h"
#include "user_interface.h"
#include "mem.h"

#define GLOBAL_DEBUG_ON

/* non sdk includes */
#include "secrets.h"
#include "messageHandler.h"
#include "mqtt.h"
#include "wifi.h"
#include "shadeControl.h"
#include "debug.h"
#include "ioPins.h"
#include "user_config.h"

// Application global
MQTT_Client mqttClient;
static void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
    if (status == STATION_GOT_IP) {
        os_printf("Wifi Connected!\r\n");
        MQTT_Connect(&mqttClient);
    }
    else {
        MQTT_Disconnect(&mqttClient);
    }
}

static void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    os_printf("MQTT: Connected\r\n");
    MQTT_Subscribe(client, DEVICE_TOPIC_STR, 0);

    MQTT_Publish(client, "/device/heartbeat", "Window Shade control Says Hello", 6, 2, 0);
    
    // turn off blue led on sonoff
    setPinState(ON_BOARD_LED_PIN, false);
}

static void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    os_printf("MQTT: Disconnected\r\n");
}

static void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    os_printf("MQTT: Published\r\n");
}

static void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len + 1),
        *dataBuf = (char*)os_zalloc(data_len + 1);
    bool success = false;
    MQTT_Client* client = (MQTT_Client*)args;
    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;
    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;
    os_printf("Received topic: %s, data: %s \r\n", topicBuf, dataBuf);
    success = handleMessage(dataBuf, data_len);
    os_printf("Handling message (which includes sending a response): returned %d\n", success);
    os_free(topicBuf);
    os_free(dataBuf);
}

static void ICACHE_FLASH_ATTR app_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    //print_info();
    
    MQTT_InitConnection(&mqttClient, "192.168.1.199", 1883, 0);

    MQTT_InitClient(&mqttClient, "shadeControl", "", "", 120, 1);
    MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
    MQTT_OnConnected(&mqttClient, mqttConnectedCb);
    MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
    MQTT_OnPublished(&mqttClient, mqttPublishedCb);
    MQTT_OnData(&mqttClient, mqttDataCb);

    // WIFI SETUP
    wifi_station_set_hostname("shadeControl");
    wifi_set_opmode_current( STATION_MODE );

    // Initialize the GPIO subsystem.
    // Apparently this just needs to be called. Odd
    gpio_init();
    
    setPinAsGpio(ON_BOARD_LED_PIN);
    // turn on blue led on nodemcu (its active low)
    setPinState(ON_BOARD_LED_PIN, true);

    WIFI_Connect(WIFI_SSID, WIFI_PASSWD, wifiConnectCb);

    os_printf("Initializing Shade Control\n");
    //initShadeControl();

    initMessage();
}


void ICACHE_FLASH_ATTR user_init(void)
{
    system_init_done_cb(app_init);
}





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
#include "si7021_sensor.h"
#include "debug.h"
#include "ioPins.h"
#include "user_config.h"
#include "queue.h"

/* The percantage of humidity that if above, the fans will kick on */
#define CHECK_SENSOR_INTERVAL_MINS 5
#define MAX_MINUTES_FAN_ON_TIME (60 * 4)
#define FAN_BELOW_THRESHOLD_TIME_MINS 20
#define HUMIDITY_DIFF_THRESHOLD 2
#define FAN_RELAY_PIN_IO_NUM 12
/*(48 hours * 60 minutes)/CHECK_SENSOR_INTERVAL_MINS */
#define NUM_HUMIDITY_SAMPLES ((48*60)/CHECK_SENSOR_INTERVAL_MINS)

typedef enum {
    e_INIT_STATE = 0,
    e_FAN_ON_STATE,
    e_FAN_POWEROFF_COUNTDOWN_STATE,
    e_FAN_OFF_STATE
} fanCtrlStateEnum;


// Application global
MQTT_Client mqttClient;

// Locals
static os_timer_t sensorCheckTimer;
static os_timer_t fanPwrDownTimer;
static unsigned numCallbacks = 0;
static unsigned turnOffCountStart = 0;
static fanCtrlStateEnum fanCtrlState = e_FAN_OFF_STATE;
static float percentHumidity = 0;
static float tempFahr = 0;
static uint8_t humiditySamples[NUM_HUMIDITY_SAMPLES] = {0};
static uint8_t averageHumidity = 0;


static void processStateMachine(void);

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
    
    // turn off blue led on nodemcu
    setPinState(ON_BOARD_LED_PIN, true);
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


bool humidityLevelOk(void){
    bool err = false;

    /* Ok, so if the humidity is greater than threshold, fans should be on.
    Fans should run for an additional 40? minutes after humidity dips below threshold.
    But what if they have been on for a really long time and haven't changed the humidity level below threshold??? We will have to see what happens.
    */

    if((percentHumidity - averageHumidity) > HUMIDITY_DIFF_THRESHOLD){
        // Humidity level too high
        os_printf("humidity percent of %d is greater than or equal threshold\n", (int)percentHumidity);
        return false;
    }
    return true;
}

static void ICACHE_FLASH_ATTR fanPwrDwn_cb(void){
    os_timer_disarm(&fanPwrDownTimer);
    os_timer_arm(&sensorCheckTimer, CHECK_SENSOR_INTERVAL_MINS*60*1000, 1);
    fanCtrlState = e_FAN_OFF_STATE;
    // Immediately call the state machine to update
    processStateMachine();
}

static void ICACHE_FLASH_ATTR startFanCountdown(void){
    os_timer_disarm(&sensorCheckTimer);
    os_timer_arm(&fanPwrDownTimer, FAN_BELOW_THRESHOLD_TIME_MINS*60*1000, 1);
}

static void ICACHE_FLASH_ATTR sensorCheckTimer_cb(void){
    os_printf("In timer callback\n");
    int i = 0;
    int numSamplesToAverage = 0;
    uint32_t samplesSum = 0;
    static uint32_t numSamplesTaken = 0;

    if(si7021GetHumidity(&percentHumidity) == false){
        os_printf("Failed to get humidity from sensor!\n");
        return;
    }

    numSamplesTaken++;

    if(numSamplesTaken > NUM_HUMIDITY_SAMPLES){
        // shift out 1 sample of data
        for(i = 0; i < NUM_HUMIDITY_SAMPLES-1; ++i){
            humiditySamples[i] = humiditySamples[i+1];
        }
        // Update the current sample
        humiditySamples[NUM_HUMIDITY_SAMPLES-1] = percentHumidity;
        numSamplesToAverage = NUM_HUMIDITY_SAMPLES;
    }
    else{
        // Havent yet filled up the container with samples.
        humiditySamples[numSamplesTaken-1] = percentHumidity;
        numSamplesToAverage = numSamplesTaken;
    }

    //os_printf("Calculating the average\n");
    for(i = 0; i < numSamplesToAverage; ++i){
        //os_printf("humiditySamples[%d] = %d\n", i, humiditySamples[i]);
        samplesSum += humiditySamples[i];
    }

    averageHumidity = samplesSum / numSamplesToAverage;
    os_printf("Ok, so our average is now: %d. from %d samples, sum of %d\n", averageHumidity, numSamplesToAverage, samplesSum);

    // 

    /*
    if(si7021GetTemperature(&tempFahr) == false){
        os_printf("Failed to get temperature from sensor!\n");
        return false;
    }
    */
    numCallbacks++;


    // State machine should probably be in its own timer... Hmmm
    return processStateMachine();
}

static void ICACHE_FLASH_ATTR processStateMachine(void){
    static bool fanTurnedOn = false;

    switch(fanCtrlState){
        case e_INIT_STATE:
            os_printf("Entering INIT_STATE\n");
            // Perform initializations
            fanCtrlState = e_FAN_OFF_STATE;
            break;
        case e_FAN_ON_STATE:
            os_printf("Entering FAN_ON_STATE\n");
            if(!fanTurnedOn){
                os_printf("Turning fan on\n");
                numCallbacks = 0;
                fanTurnedOn = true;
                setPinState(FAN_RELAY_PIN_IO_NUM, true);
                break;
            }
            // Fan already on
            else{
                if((numCallbacks * CHECK_SENSOR_INTERVAL_MINS) >= MAX_MINUTES_FAN_ON_TIME){
                    // Fan has been on for longer than max time.
                    os_printf("Fan has been on longer than max time. Setting state to fanOff\n");
                    os_printf("numCallbacks = %d. CHECK_SENSOR_INTERVAL_MINS = %d, product = %d, MAX_MINUTES_FAN_ON_TIME = %d\n", numCallbacks, CHECK_SENSOR_INTERVAL_MINS, numCallbacks*CHECK_SENSOR_INTERVAL_MINS, MAX_MINUTES_FAN_ON_TIME);
                    fanCtrlState=e_FAN_OFF_STATE;
                    break;
                }
                else if(humidityLevelOk()){
                    os_printf("Humidity level is OK, flagging fan for countdown\n");
                    fanCtrlState=e_FAN_POWEROFF_COUNTDOWN_STATE;
                    // Fall into countdown state
                }
                else{
                    break;
                }
            }
        case e_FAN_POWEROFF_COUNTDOWN_STATE:
            os_printf("Entering FAN_POWEROFF_COUNTDOWN_STATE\n");
            startFanCountdown();
            break;
        case e_FAN_OFF_STATE:
            os_printf("Entering FAN_OFF_STATE\n");
            if(fanTurnedOn){
                os_printf("Turning fan off\n");
                numCallbacks = 0;
                fanTurnedOn = false;
                setPinState(FAN_RELAY_PIN_IO_NUM, false);
            }
            else if(!humidityLevelOk()){
                os_printf("Humidity Levels not ok. Setting State to FAN_ON_STATE\n");
                fanCtrlState = e_FAN_ON_STATE;
            }
            break;
        default:
            os_printf("uhhh, unknown state. Reverting to FAN_OFF_STATE\n");
            fanCtrlState=e_FAN_OFF_STATE;
            break;
    }
}



static void ICACHE_FLASH_ATTR initSensorCallback(void){
    os_timer_disarm(&sensorCheckTimer);
    os_timer_setfn(&sensorCheckTimer, (os_timer_func_t *)sensorCheckTimer_cb, NULL);
    os_timer_setfn(&fanPwrDownTimer, (os_timer_func_t *)fanPwrDwn_cb, NULL);
    // Callback every 5 minutes (5 * 60 * 1000ms = 5mins), repeat=1
    os_timer_arm(&sensorCheckTimer, CHECK_SENSOR_INTERVAL_MINS*60*1000, 1);
}

static void ICACHE_FLASH_ATTR app_init(void)
{
    float temp, humidity;
    uart_init(BIT_RATE_115200, BIT_RATE_115200);

    os_printf("Ok, Uart initialized. Starting setup of mqtt\n");
    
    MQTT_InitConnection(&mqttClient, "test.mosquitto.org", 1883, 0);


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

    setPinAsGpio(FAN_RELAY_PIN_IO_NUM);
    
    // turn on blue led on nodemcu (its active low)
    setPinState(ON_BOARD_LED_PIN, false);
    // Initialize fan to be off.
    setPinState(FAN_RELAY_PIN_IO_NUM, false);

    WIFI_Connect(WIFI_SSID, WIFI_PASSWD, wifiConnectCb);

    os_printf("Initializing Humidity Sensor Control\n");
    if(si7021SensorInit() == false){
        os_printf("Failed to initialize si7021 sensor... Stopping\n");
        return;
    }

    if( !si7021GetTemperature(&temp)){
        os_printf("Failed to get temp\n");
    }
    else{
        os_printf("Temp is %d\n", (int)temp);
    }

    if(!si7021GetHumidity(&humidity)){
        os_printf("Failed to get humidity\n");
    }
    else{
        os_printf("Percent Humidity is %d\n", (int)humidity);
    }

    initSensorCallback();
    sensorCheckTimer_cb();
}


void ICACHE_FLASH_ATTR user_init(void)
{
    system_init_done_cb(app_init);
}





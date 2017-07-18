
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"
#include "button.h"
#include "messageHandler.h"
#include "led.h"

#define BUTTON_PRESS_DEBOUNCE_MS 120
#define BUTTON_DEPRESS_DEBOUNCE_MS 400


static uint8 buttonDebounceIndexParams[NUMBER_OF_BUTTONS] = {0};

// Topics and messages into predefined arrays so we don't have to churn
// through strings in critical sections

static const char* topics[] = {
	"/bensRoom/light1",
	"/bensRoom/light2",
	"/bensRoom/light3",
	"/bensRoom/light4"
};

static const char* toggleCommandStr = "{\"command\":\"toggle\"}";
static int messageLen = 0;

struct ButtonInfo {
	uint8 ioNum;
	uint32 ioMux;
	uint8 ioFunc;
	os_timer_t buttonDebounceTimer;
};

// just a container of button info so we can iterate through em
LOCAL struct ButtonInfo buttons[NUMBER_OF_BUTTONS] = {
	{BUTTON1_IO_NUM, BUTTON1_IO_MUX, BUTTON1_IO_FUNC, {0}},
	{BUTTON2_IO_NUM, BUTTON2_IO_MUX, BUTTON2_IO_FUNC, {0}},
	{BUTTON3_IO_NUM, BUTTON3_IO_MUX, BUTTON3_IO_FUNC, {0}},
	{BUTTON4_IO_NUM, BUTTON4_IO_MUX, BUTTON4_IO_FUNC, {0}}
};

void ICACHE_FLASH_ATTR buttonDebounceCallback(uint8* buttonIndex){
	// turn off timer
	os_timer_disarm(&buttons[*buttonIndex].buttonDebounceTimer);

	// if input still high, assume valid button press, send out message.
	// POSITVE EDGE
	if (GPIO_INPUT_GET(GPIO_ID_PIN(buttons[*buttonIndex].ioNum)) == 1 ) {
		//os_printf("DEBOUNCE: PIN%d HIGH\n", *buttonIndex+1);
		// each button index correlates to a topic
		// Eventually this will be abstracted away and configured via
		// an mqtt message... right now its all hard coded
		if(*buttonIndex == 3)
		{
			publishMessage(topics[0], toggleCommandStr, messageLen);
			publishMessage(topics[1], toggleCommandStr, messageLen);
			publishMessage(topics[2], toggleCommandStr, messageLen);
		}
		// Reading light
		else if (*buttonIndex == 2)
		{
			publishMessage("/powerStrip_SOMEHASH", "{\"command\":2,\"plugNum\":1}", 32);
		}
		// Pixel Wall
		else if(*buttonIndex == 0)
		{
			publishMessage("/powerStrip_SOMEHASH", "{\"command\":2,\"plugNum\":0}", 32);
		}

		// Now we want to be notified when the button is released
		// Setup an interrupt for falling edge to detect once we come off the button 
		gpio_pin_intr_state_set(GPIO_ID_PIN(buttons[*buttonIndex].ioNum), GPIO_PIN_INTR_NEGEDGE);
	}
	// NEGATIVE EDGE
	else { 
		//os_printf("DEBOUNCE: PIN%d LOW\n", *buttonIndex+1);
		// This is the very last step, button has been let up and back to level 0.
		// So we restore the interrupt to looking for positive edge.
		gpio_pin_intr_state_set(GPIO_ID_PIN(buttons[*buttonIndex].ioNum), GPIO_PIN_INTR_POSEDGE);
	}
}

void ICACHE_FLASH_ATTR buttonISR(void){
	uint8_t i = 0;
	// Get the register that will tell us who triggered the interrupt
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

	os_printf("ISR: gpio_status = 0x%08X\n", gpio_status);
	for(i = 0; i<NUMBER_OF_BUTTONS; ++i){
		// Check if this button generated an interrupt
		if (gpio_status & BIT(buttons[i].ioNum)){
			// disable interrupt
			gpio_pin_intr_state_set(GPIO_ID_PIN(buttons[i].ioNum), GPIO_PIN_INTR_DISABLE);
				// clear interrupt status
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(buttons[i].ioNum));

			// check input pin to see what type of interrupt we are dealing with here (posedge/negedge)
			if (GPIO_INPUT_GET(GPIO_ID_PIN(buttons[i].ioNum)) == 1 ) {
				os_printf("ISR: Got posEdge interrupt on button%d\n", i+1);
				// store the current index in a global var so we can access it from timeout function
				buttonDebounceIndexParams[i] = i;
				os_timer_setfn(&buttons[i].buttonDebounceTimer, 
								(os_timer_func_t *)buttonDebounceCallback, 
								&buttonDebounceIndexParams[i]);

				os_timer_arm(&buttons[i].buttonDebounceTimer, BUTTON_PRESS_DEBOUNCE_MS, 0);
			}

			// This is negative edge interrupt, so the button was just let go
			else{
				os_printf("ISR: Got negEdge interrupt on button%d\n", i+1);
				// store the current index in a global var so we can access it from timeout function
				buttonDebounceIndexParams[i] = i;
				os_timer_setfn(&buttons[i].buttonDebounceTimer,
								(os_timer_func_t *)buttonDebounceCallback,
								&buttonDebounceIndexParams[i]);

				os_timer_arm(&buttons[i].buttonDebounceTimer, BUTTON_DEPRESS_DEBOUNCE_MS, 0);
			}

			// we rule out the notion of two button interrupts generated at once
			break;
		}
		
	}
}

void ICACHE_FLASH_ATTR initButtons(void){
	uint8_t i = 0;
	ETS_GPIO_INTR_ATTACH(buttonISR, NULL);
	ETS_GPIO_INTR_DISABLE();

	
	// All of our buttons get the same treatment. Interrupts yo
	for(i = 0; i < NUMBER_OF_BUTTONS; ++i){
		PIN_FUNC_SELECT(buttons[i].ioMux, buttons[i].ioFunc);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(buttons[i].ioNum), 0);

		// clear register of all sources?
	    gpio_register_set(GPIO_PIN_ADDR(buttons[i].ioNum), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
	                      | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
	                      | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

	    // Set interrupt bit?
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(buttons[i].ioNum));

		// enable interrupts. Start with listening on the positive edge
		gpio_pin_intr_state_set(GPIO_ID_PIN(buttons[i].ioNum), GPIO_PIN_INTR_POSEDGE);
	}

	// do this once during init so we don't have to do it in ISR
	messageLen = strlen(toggleCommandStr);

	ETS_GPIO_INTR_ENABLE();
}
/*
void ICACHE_FLASH_ATTR timerCallback(void)
{
	static int lastSetting = 1;
    //link_led_level = (~link_led_level) & 0x01;
    //GPIO_OUTPUT_SET(GPIO_ID_PIN(BUTTON1_LED_IO_NUM), link_led_level);
    int setting = GPIO_INPUT_GET(GPIO_ID_PIN(BUTTON1_IO_NUM));
    if( (setting == 1) && (lastSetting == 0) ){
    	publishMessage();
    }
    lastSetting=setting;
    os_printf("Read %d off pin\n", setting);
}

void ICACHE_FLASH_ATTR startBlinkTimer(void){
    os_timer_disarm(&link_led_timer);
    os_timer_setfn(&link_led_timer, (os_timer_func_t *)timerCallback, NULL);
    os_timer_arm(&link_led_timer, 100, 1);
    link_led_level = 0;
    //GPIO_OUTPUT_SET(GPIO_ID_PIN(BUTTON1_LED_IO_NUM), link_led_level);
}

void ICACHE_FLASH_ATTR stopBlinkTimer(void)
{
    os_timer_disarm(&link_led_timer);
    //GPIO_OUTPUT_SET(GPIO_ID_PIN(BUTTON1_LED_IO_NUM), 0);
}*/

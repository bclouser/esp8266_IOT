
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpioPins.h"
#include "shadeControl.h"


static os_timer_t onButtonTimeout;
static const int buttonTimeoutMilliseconds = 3 * 1000; // 4 seconds
static const int stopButtonTimeoutMilliseconds = 800; // 500ms
static bool buttonDown = false;


bool ICACHE_FLASH_ATTR initShadeControl(void){
	bool ret = false;
	ret = setPinAsGpio(BUTTON_SHADE_DOWN_PIN_NUM);
	ret &= setPinAsGpio(BUTTON_SHADE_UP_PIN_NUM);
	ret &= setPinAsGpio(BUTTON_SHADE_STOP_PIN_NUM);

	// These buttons are active low. Init them to be high
	setPinState(BUTTON_SHADE_DOWN_PIN_NUM, true);
	setPinState(BUTTON_SHADE_UP_PIN_NUM, true);
	setPinState(BUTTON_SHADE_STOP_PIN_NUM, true);

	return ret;
}

static void ICACHE_FLASH_ATTR getOffAllButtons(void){
	setPinState(BUTTON_SHADE_DOWN_PIN_NUM, true);
	setPinState(BUTTON_SHADE_UP_PIN_NUM, true);
	setPinState(BUTTON_SHADE_STOP_PIN_NUM, true);
	// Turn off timer
	os_timer_disarm(&onButtonTimeout);
	buttonDown = false;
}

static void ICACHE_FLASH_ATTR shadeTimeoutCallback(void){
	os_printf("Timeout. Getting off all switches now\r\n");
	getOffAllButtons();
}

void ICACHE_FLASH_ATTR startShadeMovingUp(void){
	// Ensure we are completely off other buttons
	getOffAllButtons();

	os_printf("Pressing UP button \r\n");
	setPinState(BUTTON_SHADE_UP_PIN_NUM, false);
	os_timer_setfn(&onButtonTimeout, (os_timer_func_t *)shadeTimeoutCallback, NULL);
	os_timer_arm(&onButtonTimeout, buttonTimeoutMilliseconds, 0);
	buttonDown=true;
}

// Start shade moving without an explicit timeout
void ICACHE_FLASH_ATTR startShadeMovingDown(void){
	// Ensure we are completely off other buttons
	getOffAllButtons();

	os_printf("Pressing DOWN button \r\n");
	setPinState(BUTTON_SHADE_DOWN_PIN_NUM, false);
	os_timer_setfn(&onButtonTimeout, (os_timer_func_t *)shadeTimeoutCallback, NULL);
	os_timer_arm(&onButtonTimeout, buttonTimeoutMilliseconds, 0);
	buttonDown=true;
}

void ICACHE_FLASH_ATTR stopShade(void){
	// Ensure we are completely off other buttons
	getOffAllButtons();

	os_printf("Pressing STOP button \r\n");
	setPinState(BUTTON_SHADE_STOP_PIN_NUM, false);
	os_timer_setfn(&onButtonTimeout, (os_timer_func_t *)shadeTimeoutCallback, NULL);
	os_timer_arm(&onButtonTimeout, buttonTimeoutMilliseconds, 0);
	buttonDown=true;
}



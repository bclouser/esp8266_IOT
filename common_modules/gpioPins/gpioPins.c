#include <c_types.h>
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"


#define NUM_GPIO_PINS 16

typedef struct _pinInfo{
	const unsigned ioMux;
	const unsigned gpioFunc;
	const unsigned pwmFunc;
}pinInfo;

os_timer_t timer;

/*Definition of GPIO PIN params, for GPIO initialization*/
static const pinInfo gpioPins[NUM_GPIO_PINS]={
	{PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0},
	// GPIO1 might be wrong
	{PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1, 0},
	{PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2, 0},
	{PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3, 0},
 	{PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4, 0},
 	{PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5, 0},
 	// gpio6, 7, & 8 dont exist
 	{0, 0, 0},
 	{0, 0, 0},
 	{0, 0, 0},
 	{PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9, 0},
 	{PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10, 0},
 	// GPIO11 doesn't exist either
 	{0, 0, 0},
	{PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12, 0},
	{PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13, 0},
	{PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14, 0},
	{PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15, 0}
};

bool ICACHE_FLASH_ATTR setPinAsGpio(unsigned pinNum) {
	if(pinNum >= NUM_GPIO_PINS) {
		return false;
	}
	// TODO Check if pin can be set as gpio
	PIN_FUNC_SELECT(gpioPins[pinNum].ioMux, gpioPins[pinNum].gpioFunc);
	return true;
}

void ICACHE_FLASH_ATTR setPinState(unsigned pinNum, bool value) {
	if(pinNum >= NUM_GPIO_PINS) {
		return;
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(pinNum), value);
}

unsigned ICACHE_FLASH_ATTR readPinState(unsigned pinNum) {
	if(pinNum >= NUM_GPIO_PINS) {
		return 0;
	}
	return GPIO_INPUT_GET(GPIO_ID_PIN(pinNum));
}

bool ICACHE_FLASH_ATTR setPinAsPwm(unsigned pinNum) {
	if(pinNum >= NUM_GPIO_PINS) {
		return false;
	}
	// TODO Check if pin can be set as gpio
	PIN_FUNC_SELECT(gpioPins[pinNum].ioMux, gpioPins[pinNum].pwmFunc);
	return true;
}

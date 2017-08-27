#ifndef _GPIOPINS_H_
#define _GPIOPINS_H_

#include "ets_sys.h"
#include "gpio.h"
#include "pwm.h"

#define NUM_GPIO_PINS 16
#define PWM_DEPTH 255
#define PWM_1S 1000000

typedef struct _PinInfo{
	unsigned ioMux;
	unsigned gpioFunc;
}PinInfo;

typedef enum _BreathCtrlEnum {
	e_breathOn = 1,
	e_breathOff = 0,
	e_breathToggle = 2
}BreathCtrlEnum;


bool setPinAsGpio(unsigned pinNum);
int pwmInitPinsAsPwm(unsigned* pinList, unsigned numPins);
int pwmBreatheCtrlPin(unsigned pinIndex, BreathCtrlEnum onOffToggle);

void setPinState(unsigned pinNum, bool value);
unsigned readPinState(unsigned pinNum);

#endif
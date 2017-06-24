#ifndef _LED_H_
#define _LED_H_

#include "ets_sys.h"
#include "../gpioPins/gpioPins.h"
#include "pwm.h"


// This maps the logical to our physical pin numbers
typedef enum {
	e_ledNum1 = 2,
	e_ledNum2 = 15,
	e_ledNum3 = 13,
	e_ledNum4 = 12,
	e_numLeds
} LedNumEnum;

void initLeds();
void turnLedsOff();
bool setLed(LedNumEnum ledNum, bool on);
void  blinkLed(LedNumEnum led, int delay);

#endif
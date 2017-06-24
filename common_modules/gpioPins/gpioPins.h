#ifndef _GPIOPINS_H_
#define _GPIOPINS_H_

#include "ets_sys.h"
#include "gpio.h"
#include "pwm.h"

bool setPinAsGpio(unsigned pinNum);
bool setPinAsPwm(unsigned pinNum);

void setPinState(unsigned pinNum, bool value);
unsigned readPinState(unsigned pinNum);

#endif
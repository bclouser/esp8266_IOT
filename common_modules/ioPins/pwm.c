#include <c_types.h>
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "math.h"
#include "pwm.h"
#include "ioPins.h"
#include "error.h"

// TODO, allow for the period and duty cycle to be parameters
// NOTE: Apparently the pwm engine only ever achieves 90% duty cycle. boooo!

#define PERIOD 2000 /*us.(2ms)*/  
/* PERIOD * 1000/45 was a hard to find value that maxes out the duty cycle */    
#define MAXDUTY PERIOD * 1000/45
#define BREATHE_TIMER_INTERVAL_MS 40

typedef struct _PinBreatheInfo {
	unsigned indexNum;
	bool ascending;
	unsigned percent;
	bool enabled;
	os_timer_t timer;
}PinBreatheInfo;

// SDK pwm engine actually looks at this as 2d array of 3 unsigneds... I prefer structs.
typedef struct _PwmChannelPinInfo{
	uint32 ioMux;
	uint32 gpioFunc;
	uint32 ioNum;
}PwmChannelPinInfo;

extern const PinInfo gpioPins[NUM_GPIO_PINS];
static PinBreatheInfo breathePins[NUM_GPIO_PINS] = {0};

int ICACHE_FLASH_ATTR pwmInitPinsAsPwm(unsigned* pinList, unsigned numPins){
	int i = 0;
	unsigned pinNum = 0;
	PwmChannelPinInfo pwmChannelInfo[PWM_CHANNEL_NUM_MAX] = {0, 0, 0};
	uint32 pwmDutyInit[PWM_CHANNEL_NUM_MAX] = {0};

	if(pinList == NULL){
		return -EINVAL;
	}
	if(numPins > PWM_CHANNEL_NUM_MAX){
		os_printf("Requested number of pins to use as pwm is more than available\n");
		return -E2BIG;
	}
	for(i = 0; i < numPins; ++i)
	{
		pinNum = pinList[i];
		// Fill in info for initializing pwm engine
		pwmChannelInfo[i].ioMux = gpioPins[pinNum].ioMux;
		pwmChannelInfo[i].gpioFunc = gpioPins[pinNum].gpioFunc;
		pwmChannelInfo[i].ioNum = pinNum;

		// Configure gpio setting on the pin
		PIN_FUNC_SELECT(gpioPins[pinNum].ioMux, gpioPins[pinNum].gpioFunc);
		// turn it off to start
		GPIO_OUTPUT_SET(GPIO_ID_PIN(pinNum), 0);
	
		// Initialize structs for breathing functionality
		breathePins[i].indexNum = i;
		breathePins[i].ascending = true;
		breathePins[i].percent = 0;
		breathePins[i].enabled = false;
	}
    
    pwm_init(PERIOD, pwmDutyInit, numPins, (uint32 (*)[3])&pwmChannelInfo);
    pwm_start();
    return 0;
}

static int ICACHE_FLASH_ATTR pwmSetPin(unsigned pwmChannel, uint8 dutyPercent){
	if(pwmChannel < 0 || pwmChannel > PWM_CHANNEL_NUM_MAX){
		return -EINVAL;
	}
	if( (dutyPercent < 0) || (dutyPercent > 100) ){
		return -EINVAL;
	}
	//os_printf("User requested dutyPercent %d\n", dutyPercent);
	// need to convert percentage to actual duty value
	double decimalPercent = ((double)dutyPercent)/100;
	double actualDuty = decimalPercent * (double)MAXDUTY;

	uint32 dutyToSet = (uint32)actualDuty;
	//os_printf("Actual actualDuty = %.2f\n", actualDuty);
	//os_printf("Actual dutyToSet = %d\n", dutyToSet);
	//os_printf("setting pwm channel: %d\n", pwmChannel);
	pwm_set_duty(dutyToSet, pwmChannel);
	pwm_start(); // start must be called after every change
	return 0;
}

int ICACHE_FLASH_ATTR pwmBreatheCtrlPin(unsigned pinIndex, BreathCtrlEnum onOffToggle){
	int err = 0;

	//os_printf("pwmBreatheCtrlPin: pinIndex = %d, onOffToggle = %d\n", pinIndex, onOffToggle);
	if(pinIndex < 0 || pinIndex > NUM_GPIO_PINS){
		return -EINVAL;
	}
	// First verify that duty is within acceptable range
	// For now, we error out, in the future we could clip to min, max
	if( (onOffToggle < 0) || (onOffToggle > 100)){
		return -EINVAL;
	}
	switch(onOffToggle){
		case e_breathOff:
			// OFF
			breathePins[pinIndex].enabled = false;
			err = pwmSetPin(pinIndex, 0);
			os_timer_disarm(&breathePins[pinIndex].timer);
			os_delay_us(2000); // TODO: figure out why delay is needed
			break;
		case e_breathOn:
			// ON
			breathePins[pinIndex].enabled = true;
			err = pwmSetPin(pinIndex, 0);
			os_timer_arm(&breathePins[pinIndex].timer, BREATHE_TIMER_INTERVAL_MS, 1);
			break;
		case e_breathToggle:
			// TOGGLE
			if(breathePins[pinIndex].enabled){
				err = pwmSetPin(pinIndex, 0);
				os_delay_us(2000); // TODO: figure out why delay is needed
				os_timer_disarm(&breathePins[pinIndex].timer);
			}
			else{
				os_timer_arm(&breathePins[pinIndex].timer, BREATHE_TIMER_INTERVAL_MS, 1);
			}
			breathePins[pinIndex].enabled = !breathePins[pinIndex].enabled;
			break;
		default:
			os_printf("Bad value recieved for onOffToggle should be 0, 1, or 2. Got: %u\n", onOffToggle);
			return -EINVAL;
	}
	return err;
}

static void ICACHE_FLASH_ATTR pwmBreath_cb(PinBreatheInfo* pin){
	if(pin == NULL){
		return;
	}
	//os_printf("pwm cb: %d", pin->indexNum);
	if(pin->ascending && (pin->percent >= 100)){
		// Currently ascending and reached top. Change direction
		pin->ascending = false;
	}
	else if(!pin->ascending && (pin->percent <= 0)){
		// Currently descending and at 0. Change direction
		pin->ascending = true;
	}
	else{	
		// only modify pins that are enabled
		if(pin->enabled){
			pwmSetPin(pin->indexNum, pin->percent);
			if(pin->ascending){
				++pin->percent;
			}
			else{
				--pin->percent;
			}
		}
	}
}

void ICACHE_FLASH_ATTR pwmStartBreath(unsigned pinIndex, int initVal){
	// TODO Verify pinNum is valid
	breathePins[pinIndex].enabled = true;
	breathePins[pinIndex].percent = initVal;
	os_timer_disarm(&breathePins[pinIndex].timer);
	os_timer_setfn(&breathePins[pinIndex].timer, (os_timer_func_t *)pwmBreath_cb, &breathePins[pinIndex]);
	os_timer_arm(&breathePins[pinIndex].timer, BREATHE_TIMER_INTERVAL_MS, 1);
}

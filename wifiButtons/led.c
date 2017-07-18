#include <c_types.h>
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "led.h"
#include "math.h"




#define PERIOD 2000 /*us.(2ms)*/  
/* PERIOD * 1000/45 was a hard to find value that maxes out the duty cycle */    
#define MAXDUTY PERIOD * 1000/45

struct LedInfo {
	LedNumEnum num;
	bool ascending;
	int percent;
	bool enabled;
	os_timer_t timer;
};

LOCAL struct LedInfo leds[e_numLeds] = {0};

bool ICACHE_FLASH_ATTR enableDisableLed(LedNumEnum ledNum, uint8 onOffToggle)
{
	if(ledNum < 0 || ledNum > e_numLeds){
		return false;
	}
	// First verify that duty is within acceptable range
	// For now, we error out, in the future we could clip to min, max
	if( (onOffToggle < 0) || (onOffToggle > 100)){
		return false;
	}

	switch(onOffToggle)
	{
		case 0:
			leds[ledNum].enabled = false;
			setLed(ledNum, 0);
			break;
		case 1:
			leds[ledNum].enabled = true;
			setLed(ledNum, 0);
			break;
		case 2:
			if(leds[ledNum].enabled)
			{
				setLed(ledNum, 0);
			}
			leds[ledNum].enabled = !leds[ledNum].enabled;
			break;
		default:
			os_printf("Bad value recieved for onOffToggle should be 0, 1, or 2. Got: %u\n", onOffToggle);
			break;
	}
}

bool ICACHE_FLASH_ATTR setLed(LedNumEnum ledNum, uint8 dutyPercent)
{
	if(ledNum < 0 || ledNum > e_numLeds){
		return false;
	}
	// First verify that duty is within acceptable range
	// For now, we error out, in the future we could clip to min, max
	if( (dutyPercent < 0) || (dutyPercent > 100) ){
		return false;
	}

	//os_printf("Setting LED %d\n", ledNum);
	// need to convert percentage to actual duty value

	double decimalPercent = ((double)dutyPercent)/100;
	double actualDuty = decimalPercent * (double)MAXDUTY;

	uint32 dutyToSet = (uint32)actualDuty;
	//os_printf("Actual actualDuty = %.2f\n", actualDuty);
	//os_printf("Actual dutyToSet = %d\n", dutyToSet);
	pwm_set_duty( dutyToSet, (uint8)ledNum );
	pwm_start(); // start must be called after every change
}

LOCAL void ICACHE_FLASH_ATTR ledBreath_cb(struct LedInfo* led)
{
	if(led->ascending && (led->percent >= 100))
	{
		// Currently ascending and reached top. Change direction
		led->ascending = false;
	}
	else if(!led->ascending && (led->percent <= 0))
	{
		// Currently descending and at 0. Change direction
		led->ascending = true;
	}
	else
	{	
		// only modify leds that are enabled
		if(led->enabled)
		{
			setLed(led->num, led->percent);
			if(led->ascending)
			{
				++led->percent;
			}
			else
			{
				--led->percent;
			}
		}
	}
}

void ICACHE_FLASH_ATTR startBreath(LedNumEnum led, int initVal)
{
	leds[led].percent = initVal;
	os_timer_disarm(&leds[led].timer);
	os_timer_setfn(&leds[led].timer, (os_timer_func_t *)ledBreath_cb, &leds[led]);
	os_timer_arm(&leds[led].timer, 30, 1);
}

void ICACHE_FLASH_ATTR initLeds()
{
	// initialize led info structs
	int i = 0;
	for(i = 0; i < e_numLeds; ++i)
	{
		leds[i].num = i;
		leds[i].ascending = true;
		leds[i].percent = 0;
		leds[i].enabled = true;
	}

	os_printf("Initializing leds\n");

	PIN_FUNC_SELECT(PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC);
	PIN_FUNC_SELECT(PWM_1_OUT_IO_MUX, PWM_1_OUT_IO_FUNC);
	PIN_FUNC_SELECT(PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC);
	PIN_FUNC_SELECT(PWM_3_OUT_IO_MUX, PWM_3_OUT_IO_FUNC);

	/* We may want to change this in the future, right now we are only using 2 pins for pwm */
    uint32 io_info[][3] = {   {PWM_0_OUT_IO_MUX,PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM},
                              {PWM_1_OUT_IO_MUX,PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM},
                              {PWM_2_OUT_IO_MUX,PWM_2_OUT_IO_FUNC,PWM_2_OUT_IO_NUM},
                              {PWM_3_OUT_IO_MUX,PWM_3_OUT_IO_FUNC,PWM_3_OUT_IO_NUM}
                         };
    
    // Initialize duty cycle for channels 0 and 1 to 0
    uint32 pwm_duty_init[] = {MAXDUTY/2, MAXDUTY};
    pwm_init(PERIOD, pwm_duty_init, 4, io_info);
    pwm_start(); 
}
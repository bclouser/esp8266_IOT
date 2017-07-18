#ifndef SERVO
#define SERVO

#include "ets_sys.h"
#include "gpio.h"
#include "pwm.h"


/*Definition of GPIO PIN params, for GPIO initialization*/
#define PWM_0_OUT_IO_MUX     PERIPHS_IO_MUX_GPIO2_U
#define PWM_0_OUT_IO_NUM     2
#define PWM_0_OUT_IO_FUNC    FUNC_GPIO2

#define PWM_1_OUT_IO_MUX     PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM     15
#define PWM_1_OUT_IO_FUNC    FUNC_GPIO15

#define PWM_2_OUT_IO_MUX     PERIPHS_IO_MUX_MTCK_U
#define PWM_2_OUT_IO_NUM     13
#define PWM_2_OUT_IO_FUNC    FUNC_GPIO13

#define PWM_3_OUT_IO_MUX     PERIPHS_IO_MUX_MTDI_U
#define PWM_3_OUT_IO_NUM     12
#define PWM_3_OUT_IO_FUNC    FUNC_GPIO12




#define PWM_DEPTH 255
#define PWM_1S 1000000


typedef enum {
	e_ledNum1 = 0,
	e_ledNum2,
	e_ledNum3,
	e_ledNum4,
	e_numLeds
} LedNumEnum;

void initLeds();
bool setLed(LedNumEnum ledNum, uint8 dutyPercent);
bool enableDisableLed(LedNumEnum ledNum, uint8 onOffToggle);

#endif
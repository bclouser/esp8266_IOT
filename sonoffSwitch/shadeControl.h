#ifndef _SHADE_CONTROL_H_
#define _SHADE_CONTROL_H_

#define BUTTON_SHADE_STOP_PIN_NUM 4
#define BUTTON_SHADE_DOWN_PIN_NUM   5
#define BUTTON_SHADE_UP_PIN_NUM 14

bool initShadeControl(void);
void startShadeMovingUp(void);
// Start shade moving without an explicit timeout
void startShadeMovingDown(void);
void stopShade(void);

#endif
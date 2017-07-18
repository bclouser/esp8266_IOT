#ifndef _FAN_CONTROL_H_
#define _FAN_CONTROL_H_

#define IR_PIN_NUM 4
#define TX_PIN_NUM 5

bool initFanControl(void);
void sendFanPwrSpeedToggleCmd(void);

#endif
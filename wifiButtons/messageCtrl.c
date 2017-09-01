#include "osapi.h"

#include "messageHandler.h"
#include "led.h"
#include "user_config.h"
#include "error.h"
#include "ioPins.h"

static int ICACHE_FLASH_ATTR msgCallback(int command);
static MessageHandler msgHandler = {
	.func = msgCallback,
	.cmds = {"command", {0x00}, 0},
	.statusTopicString = DEVICE_TOPIC_STATUS,
};

static int ICACHE_FLASH_ATTR msgCallback(int cmd) {
	int err = 0;
	os_printf("BEN SAYS: Hey, we made it to the msgCallback with command: %d\n", cmd);
	switch(cmd){
		case 0:
			err = pwmBreatheCtrlPin(e_ledNum1, e_breatheOff);
			err |= pwmBreatheCtrlPin(e_ledNum2, e_breatheOff);
			err |= pwmBreatheCtrlPin(e_ledNum3, e_breatheOff);
			err |= pwmBreatheCtrlPin(e_ledNum4, e_breatheOff);
			return err;
			break;
		case 1:
			err = pwmBreatheCtrlPin(e_ledNum1, e_breatheOn);
			err |= pwmBreatheCtrlPin(e_ledNum2, e_breatheOn);
			err |= pwmBreatheCtrlPin(e_ledNum3, e_breatheOn);
			err |= pwmBreatheCtrlPin(e_ledNum4, e_breatheOn);
			return err;
			break;
		case 2:
			err = pwmBreatheCtrlPin(e_ledNum1, e_breatheToggle);
			err |= pwmBreatheCtrlPin(e_ledNum2, e_breatheToggle);
			err |= pwmBreatheCtrlPin(e_ledNum3, e_breatheToggle);
			err |= pwmBreatheCtrlPin(e_ledNum4, e_breatheToggle);
			return err;
			break;
		default:
			os_printf("Bad value parsed for button value\n");
			return -EINVAL;
			break;
	}
	return -ENOSYS;
}

void ICACHE_FLASH_ATTR initMessage() {
	initMessageHandler(&msgHandler);
}




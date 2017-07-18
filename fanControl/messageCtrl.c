#include "osapi.h"

#include "messageHandler.h"
#include "fanControl.h"
#include "user_config.h"
#include "error.h"

static int ICACHE_FLASH_ATTR msgCallback(int command);
static MessageHandler msgHandler = {
	.func = msgCallback,
	.cmds = {"update", {0x00}, 0},
	.statusTopicString = DEVICE_TOPIC_STATUS,
};


static int ICACHE_FLASH_ATTR msgCallback(int command) {
	os_printf("BEN SAYS: Hey, we made it to the msgCallback with command: %d\n", command);
	switch(command){
		case 1:
			sendFanPwrSpeedToggleCmd();
			return 0;
		default:
			os_printf("Bad fan control command received\n");
			break;
	}
	return -ENOSYS;
}

void ICACHE_FLASH_ATTR initMessage() {
	initMessageHandler(&msgHandler);
}




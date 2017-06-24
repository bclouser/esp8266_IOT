#include "osapi.h"

#include "messageHandler.h"
#include "shadeControl.h"

static bool ICACHE_FLASH_ATTR msgCallback(int command);
static MessageHandler msgHandler = {
	.func = msgCallback,
	.cmds = {"update", {0x00}, 0},
};


static bool ICACHE_FLASH_ATTR msgCallback(int command) {
	os_printf("BEN SAYS: Hey, we made it to the msgCallback\n");
	return true;
}

void ICACHE_FLASH_ATTR initMessage() {
	initMessageHandler(&msgHandler);
}




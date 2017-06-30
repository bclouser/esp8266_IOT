#include "osapi.h"

#include "messageHandler.h"
#include "epson.h"
#include "user_config.h"
#include "error.h"

static int ICACHE_FLASH_ATTR msgCallback(int command);
static MessageHandler msgHandler = {
	.func = msgCallback,
	.cmds = {"update", {0x00}, 0},
	.statusTopicString = DEVICE_TOPIC_STATUS,
};


static int ICACHE_FLASH_ATTR msgCallback(int command) {
	PowerStateEnum pwrState;
	ProjectorErrorEnum projectorError;
	int filterUseTime = 0;
	os_printf("BEN SAYS: Hey, we made it to the msgCallback with command: %d\n", command);
	switch(command){
		case 0:
			os_printf("Powering off projector...\n");
			return Epson_PowerOff();
		case 1:
			os_printf("Powering on projector...\n");
			return Epson_PowerOn();
		case 2:
			os_printf("Getting power state of projector...\n");
			return Epson_GetPowerState(&pwrState);
		case 3: 
			os_printf("Getting projector filter use time...\n");
			return Epson_GetFilterUseTime(&filterUseTime);
		case 4:
			os_printf("Getting projector error status...\n");
			return Epson_GetErrors(&projectorError);
		case 5: // Volume Up
			os_printf("Turning the projector volume up one...\n");
			return Epson_VolumeChange(e_volumeUp);
		case 6: // Volume Down
			os_printf("Turning the projector volume down one...\n");
			return Epson_VolumeChange(e_volumeDown);
		default:
			return -ENOSYS;
	}
}

void ICACHE_FLASH_ATTR initMessage() {
	initMessageHandler(&msgHandler);
}



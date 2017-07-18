#include "osapi.h"

#include "messageHandler.h"
#include "led.h"
#include "user_config.h"
#include "error.h"

static int ICACHE_FLASH_ATTR msgCallback(int command);
static MessageHandler msgHandler = {
	.func = msgCallback,
	.cmds = {"update", {0x00}, 0},
	.statusTopicString = DEVICE_TOPIC_STATUS,
};

/*
if( type == JSON_TYPE_PAIR_NAME ){
			if( (jsonparse_strcmp_value(&js, "buttonLed1") == 0) ||
				(jsonparse_strcmp_value(&js, "buttonLed2") == 0) ||
				(jsonparse_strcmp_value(&js, "buttonLed3") == 0) ||
				(jsonparse_strcmp_value(&js, "buttonLed4") == 0) )
			{
				cmdKeyLen = jsonparse_copy_value(&js, buf, 32);
				// dirty hack to convert last character to integer (ascii only)
				buttonNum = (int)(buf[10-1] - 48);
				//os_printf("Command Key received: %s. Length = %d, number = %d\n", buf, cmdKeyLen, buttonNum);

				type = jsonparse_next(&js);
				// ok, so now look at the value
				type = jsonparse_next(&js);
				if( type == JSON_TYPE_STRING){
					jsonparse_copy_value(&js, buf, 32);
				}
				else if(type == JSON_TYPE_INT || JSON_TYPE_NUMBER){
					jsonparse_copy_value(&js, buf, 32);
					uint8 onOffToggle = jsonparse_get_value_as_int(&js);

				}
				else{
					os_printf("BAD PARSE! Couldn't find value associated with key!\n");
				}
			}
*/

static int ICACHE_FLASH_ATTR msgCallback(int buttonNum) {
	os_printf("BEN SAYS: Hey, we made it to the msgCallback with command: %d\n", command);
	switch(buttonNum){
		case 1:
			enableDisableLed(e_ledNum1, onOffToggle);
			break;
		case 2:
			enableDisableLed(e_ledNum2, onOffToggle);
			break;
		case 3:
			enableDisableLed(e_ledNum3, onOffToggle);
			break;
		case 4:
			enableDisableLed(e_ledNum4, onOffToggle);
			break;
		default:
			os_printf("Bad value parsed for button value\n");
			break;
	}
	return -ENOSYS;
}

void ICACHE_FLASH_ATTR initMessage() {
	initMessageHandler(&msgHandler);
}




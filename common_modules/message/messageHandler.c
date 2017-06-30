//#include <queue.h>
#include "messageHandler.h"
#include "json/jsonparse.h"
#include "json/jsontree.h"
#include "osapi.h"
#include "mqtt.h"
#include "error.h"


extern MQTT_Client mqttClient;
static MessageHandler* msgHandler = NULL;

/*struct QueueEntry {
	int btnIndex;
	STAILQ_ENTRY(stailq_data_s) entries;
};
*/

static const char* errorStrings[MAX_NUM_ERRORS] =
{
	"",
	"EPERM",
	"EIO",
	"ENXIO",
	"E2",
	"EAGAIN",
	"ENOMEM",
	"EACCES",
	"EFAULT",
	"ENOTBLK",
	"EBUSY",
	"ENODEV",
	"EINVAL",
	"ENOSPC",
	"EROFS",
	"EDOM",
	"ERANGE",
	"ENOSYS",
	"ENODATA",
	"ETIME",
	"EPROTO",
	"EMSGSIZE"
};

static const unsigned errStrLen=12;
static const char* errLookup(int errno){
	int posErrno = (-1)*errno;
	if( (errno >= 0) || (posErrno>MAX_NUM_ERRORS) ){
		return "";
	}
	return errorStrings[posErrno];
}

//STAILQ_HEAD(MessageQueue, QueueEntry) g_messageQueue;

int ICACHE_FLASH_ATTR initMessageHandler(MessageHandler* messageHandler){
	// initialize our queue!
	//STAILQ_INIT(&g_messageQueue); /* Initialize the queue. */

	if(messageHandler == NULL)
	{
		os_printf("Bad messageHandler struct passed in\n");
		return -1;
	}

	msgHandler = messageHandler;
	return 0;
}

bool ICACHE_FLASH_ATTR handleMessage(const char* messageBuf, const uint32_t len){
	struct jsonparse_state js;
	jsonparse_setup(&js, messageBuf, len);
	//int duty = jsonparse_get_value_as_int(js);
	char buf[32] = {0};
	int type = 0;
	int err = 0;
	while( (type = jsonparse_next(&js))!=JSON_TYPE_ERROR ){
		switch(type){
			case JSON_TYPE_ARRAY:
				os_printf("type = JSON_TYPE_ARRAY\n");
				break;
			case JSON_TYPE_OBJECT:
				os_printf("type = JSON_TYPE_OBJECT\n");
				break;
			case JSON_TYPE_PAIR:
				os_printf("type = JSON_TYPE_PAIR\n");
				break;
			case JSON_TYPE_PAIR_NAME:
				os_printf("type = JSON_TYPE_PAIR_NAME\n");
				break;
			case JSON_TYPE_STRING:
				os_printf("type = JSON_TYPE_STRING\n");
				break;
			case JSON_TYPE_INT:
				os_printf("type = JSON_TYPE_INT\n");
				break;
			case JSON_TYPE_NUMBER:
				os_printf("type = JSON_TYPE_NUMBER\n");
				break;
			case JSON_TYPE_ERROR:
				os_printf("type = JSON_TYPE_ERROR\n");
				break;
			case JSON_TYPE_NULL:
				os_printf("type = JSON_TYPE_NULL\n");
				break;
			case JSON_TYPE_TRUE:
				os_printf("type = JSON_TYPE_TRUE\n");
				break;
			case JSON_TYPE_FALSE:
				os_printf("type = JSON_TYPE_FALSE\n");
				break;
			case JSON_TYPE_CALLBACK:
				os_printf("type = JSON_TYPE_CALLBACK\n");
				break;
			default:
			 	os_printf("unknown type\n");
		}

		//snowden/buildingDoorControl {"command":1|0};
		// Found a key-value pair!
		if( type==JSON_TYPE_PAIR_NAME ){
			if( jsonparse_strcmp_value(&js, "command")==0 ){
				os_printf("Ok, we found command key\n");
				//cmdKeyLen = jsonparse_copy_value(&js, buf, 32);
				type = jsonparse_next(&js); // this will be the colon
				// ok, so now look at the value
				type = jsonparse_next(&js);
				if( type == JSON_TYPE_STRING){
					jsonparse_copy_value(&js, buf, 32);
				}
				else if( (type == JSON_TYPE_INT) || JSON_TYPE_NUMBER ){
					jsonparse_copy_value(&js, buf, 32);
					uint8 shadeCommand = jsonparse_get_value_as_int(&js);

					// Actually call the associated function
					os_printf("Calling associated function\n");
					err = msgHandler->func(shadeCommand);

					if(!err) {
						os_printf("BEN SAYS: performing requested command was successful\n");
						return publishMessage(msgHandler->statusTopicString, messageBuf, len);
					}
					else {
						os_printf("BEN SAYS: Failed to perform requested command\n");
						return publishMessage(msgHandler->statusTopicString, errLookup(err), errStrLen);
					}
				}
				else{
					os_printf("BAD PARSE! Couldn't find value associated with key!\n");
				}
			}
			else
			{
				os_printf("Hmmm, key value was not a string\n");
				return false;
			}
		}
	}
	return true;
}

bool ICACHE_FLASH_ATTR publishMessage(const char* topic, const char* message, int len){
	return MQTT_Publish(&mqttClient, topic, message, len, 1, 0);
}


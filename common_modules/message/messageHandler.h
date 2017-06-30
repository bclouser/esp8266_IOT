#ifndef MESSAGE_HANDLER
#define MESSAGE_HANDLER

#include "ets_sys.h"

#define MAX_NUM_CMDS 128
#define MAX_NUM_ARGS 8


typedef struct _Command {
	const char* commandString;
	int argsList[MAX_NUM_ARGS];
	int numCmds;
}Command;

typedef struct _MessageHandler {
	int (*func)(int);
	Command cmds[MAX_NUM_CMDS];
	const char* statusTopicString;
}MessageHandler;

// pass in list of commands and the number of arguments with types?

int initMessageHandler(MessageHandler* messageHandler);

bool handleMessage(const char* messageBuf, const uint32_t len);

bool publishMessage(const char* topic, const char* message, int len);


#endif
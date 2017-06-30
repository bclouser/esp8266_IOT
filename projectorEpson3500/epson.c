#include <stdlib.h>
#include "osapi.h"

#include "epson.h"
#include "serial.h"

static char readBuf[64] = {0};


int Epson_init(void){
	serial_flushRx();
	// Try to talk to projector and see if it responds as we expect
	serial_write("\r");
	os_delay_us(10000); // 0.01 second
	// we anticipate timeout
	int numBytes = serial_read(readBuf, 16);

	os_printf("Read back %d bytes\n", numBytes);
	os_printf("readBuf[0] = %c and readBuf[1] = %c\n", readBuf[0], readBuf[1]);

	if(numBytes != 2){
		return -1;
	}
	// Expecting to get a single colon response
	if( ((int)readBuf[0] != (int)'\r') && ((int)readBuf[1] != (int)':') ){
		return -1;
	}
	return 0;
}


int Epson_GetPowerState(PowerStateEnum* pwrState){
	int rawPwrState = 0;
	// Get rid of any leftover junk in our rx buffer
	serial_flushRx();

	serial_write("PWR?\r");
	// ReadBack PWR=XX
	int bytes = serial_read(readBuf, 64);
	if (bytes <= 0){
		os_printf("We didn't read any data\n");
		return -1;
	}

	readBuf[bytes] = '\0';
	os_printf("Read back %d bytes\n", bytes);
	os_printf("Buffer looks like: %s\n", readBuf);
	
	if(strncmp("PWR", readBuf, 3) != 0){
		os_printf("Bad or invalid response\n");
		return -1;
	}

	rawPwrState = atoi(&readBuf[4]);
	os_printf("Converting power code to int = %d\n", rawPwrState);
	*pwrState = (PowerStateEnum)rawPwrState;
	return 0;
}

int Epson_PowerOn(void){
	// Get rid of any leftover junk in our rx buffer
	serial_flushRx();
	serial_write("PWR ON\r");
	serial_flushRx();
}

int Epson_PowerOff(void){
	// Get rid of any leftover junk in our rx buffer
	serial_flushRx();
	serial_write("PWR OFF\r");
	serial_flushRx();
}

int Epson_GetFilterUseTime(int* useTime){
	// Get rid of any leftover junk in our rx buffer
	serial_flushRx();

	serial_write("FILTER?\r");
	int bytes = serial_read(readBuf, 12);
	os_printf("Read back %d bytes\n", bytes);
	readBuf[bytes] = '\0';
	os_printf("Buffer looks like: %s\n", readBuf);

	if(strncmp("ERR", readBuf, 3) != 0){
		os_printf("Bad or invalid response\n");
		return -1;
	}

	*useTime = atoi(&readBuf[4]);
	os_printf("Converting power code to int = %d\n", *useTime);
	serial_flushRx();
	return 0;
}

int Epson_GetErrors(ProjectorErrorEnum* error){
	int rawError = 0;
	// Get rid of any leftover junk in our rx buffer
	serial_flushRx();

	serial_write("ERR?\r");
	int bytes = serial_read(readBuf, 12);
	os_printf("Read back %d bytes\n", bytes);
	readBuf[bytes] = '\0';
	os_printf("Buffer looks like: %s\n", readBuf);

	if(strncmp("ERR", readBuf, 3) != 0){
		os_printf("Bad or invalid response\n");
		return -1;
	}

	rawError = atoi(&readBuf[4]);
	os_printf("Converting power code to int = %d\n", rawError);
	*error = (ProjectorErrorEnum)rawError;
	serial_flushRx();
	return 0;
}

int Epson_VolumeChange(VolumeDirection direction){
	// Get rid of any leftover junk in our rx buffer
	serial_flushRx();
	if(direction == e_volumeUp){
		serial_write("VOL INC\r");
	}
	else if(direction == e_volumeDown){
		serial_write("VOL DEC\r");
	}
	else{
		os_printf("value volume specified\n");
	}
	serial_flushRx();
	return 0;
}





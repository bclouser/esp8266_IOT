
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "math.h"
#include "driver/i2c_master.h"
#include "si7021_sensor.h"

#define SI7021_MAX_WAIT_ACK_ATTEMPTS 120

bool ICACHE_FLASH_ATTR si7021SensorInit(void){

	i2c_master_gpio_init();
	// Send some stops to clean the bus.
	i2c_master_stop();
	i2c_master_stop();
	return true;
}

bool ICACHE_FLASH_ATTR si7021MeasCmdNoHold(uint8 cmd, uint16* out){
	uint8 addr = SI7021_I2C_BUS_ADDR;
	uint8 nack = 0;
	int readAttempts = SI7021_MAX_WAIT_ACK_ATTEMPTS;

	// TODO, validate cmd is in fact a measurement command

	// Send Start
	i2c_master_start();
	
	i2c_master_writeByte(addr<<1);
	nack = i2c_master_getAck();

	if (nack) {
	    os_printf("si7021 I2C_ERR: addr did not ack in response to setup write\n");
	    i2c_master_stop();
	    return false;
	}

	// Send over command to read
	i2c_master_writeByte(cmd);
	nack = i2c_master_getAck();

	if (nack) {
	    os_printf("si7021 I2C_ERR: addr did not ack in response to cmd\n");
	    i2c_master_stop();
	    return false;
	}

	readAttempts = SI7021_MAX_WAIT_ACK_ATTEMPTS;
	nack = 1;

	// Continually perform a read, waiting for an ack.
	// Device will send NACKS until its ready
	while(readAttempts-- && nack){
		os_delay_us(2);
		i2c_master_start();
		i2c_master_writeByte((SI7021_I2C_BUS_ADDR << 1) +1);
		nack = i2c_master_getAck();
	}

	if((readAttempts <= 0)){
		os_printf("si7021 I2C_ERR: Timed out waiting for an ack after issuing cmd\n");
		return false;
	} 

	// Ok, so we got an ACK, should have a MSB of data. Lets clock it in!
	*out = 0x0000;
	*out = i2c_master_readByte() << 8;

	// Tell the slave we are ready for the LSB by sending an ack
	i2c_master_setAck(0);

	// Clock in the LSB of data
	*out |= i2c_master_readByte();

	// We actually expect a nack to follow the LSB to indicate the end of transmission.
	nack = i2c_master_getAck();
	if (!nack) {
	    os_printf("si7021 I2C_ERR: Expecting a nack, got ack, so this is bad!\n");
	    i2c_master_stop();
	    return false;
	}

	i2c_master_stop();
	return true;
}


bool ICACHE_FLASH_ATTR si7021GetTemperature(float* tempFahr){
	uint16 rawTemp = 0;

	if(si7021MeasCmdNoHold(CMD_MEAS_TEMP_NO_HOLD, &rawTemp) == false){
		return false;
	}
	// Calc temperature from the raw value provided from the chip
	*tempFahr = (float)((TEMP_CALC_CONST * (float)rawTemp)/TEMP_CALC_DIVISOR) - 46.85;
	// Actually make it fahrenheit...
	*tempFahr = (*tempFahr * 1.8) + 32;
	return true;
}

bool ICACHE_FLASH_ATTR si7021GetHumidity(float* humidity){
	uint16 rawHumidity = 0;

	if(si7021MeasCmdNoHold(CMD_MEAS_HUMIDITY_NO_HOLD, &rawHumidity) == false){
		return false;
	}
	// Calc humidty from the raw value provided from the chip
	*humidity = (float)((HUMIDITY_CALC_CONST * (float)rawHumidity)/HUMIDITY_CALC_DIVISOR) - 6;

	if(*humidity > 100){
		*humidity = 100;
	}
	else if(*humidity < 0){
		*humidity = 0;
	}
	return true;
}
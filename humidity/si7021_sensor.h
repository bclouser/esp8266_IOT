#ifndef _FAN_CONTROL_H_
#define _FAN_CONTROL_H_

#define SI7021_I2C_BUS_ADDR 0x40




bool si7021SensorInit();
bool si7021GetTemperature(float* tempFahr);
bool si7021GetHumidity(float* humidity);



/* Measure Relative Humidity, Hold Master Mode */
#define CMD_MEAS_HUMIDITY_WITH_HOLD 0xE5
/* Measure Relative Humidity, No Hold Master Mode */
#define CMD_MEAS_HUMIDITY_NO_HOLD 0xF5
/* Measure Temperature, Hold Master Mode */
#define CMD_MEAS_TEMP_WITH_HOLD 0xE3
/* Measure Temperature, No Hold Master Mode */
#define CMD_MEAS_TEMP_NO_HOLD 0xF3
/* Read Temperature Value from Previous RH Measurement */
#define CMD_READ_TEMP_PREV 0xE0
/* Reset */
#define CMD_RESET 0xFE
/* Write RH/T User Register 1 */
#define CMD_WRITE_RHT_USER_REG 0xE6
/* Read RH/T User Register 1 */
#define CMD_READ_RHT_USER_REG 0xE7
/* Write Heater Control Register */
#define CMD_WRITE_HEAT_CTRL_REG 0x51
/* Read Heater Control Register */
#define CMD_READ_HEAT_CTRL_REG 0x11
/* Read Electronic ID 1st Byte */
/* 0xFA 0x0F */
/* Read Electronic ID 2nd Byte */
/* 0xFC 0xC9 */
/* Read Firmware Revision */
/* 0x84 0x */



#define TEMP_CALC_DIVISOR 65536
#define TEMP_CALC_CONST 175.72
#define HUMIDITY_CALC_DIVISOR 65536
#define HUMIDITY_CALC_CONST 125





#endif
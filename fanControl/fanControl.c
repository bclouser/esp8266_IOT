
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "ioPins.h"
#include "fanControl.h"

#define NUM_BITS_SPEED_TOGGLE_CMD 47

typedef enum _SampleTypeEnum{
	e_typeMark,
	e_typeSpace,
}SampleTypeEnum;

typedef struct _Sample {
	unsigned time_us;
	SampleTypeEnum type;
}Sample;

static const maxSamples = 200;
static Sample samples[200] = {0};
static numSamples = 0;

void collectData(void){
	unsigned timeSinceLastMark_us = 0;
	unsigned timeSinceLastSpace_us = 0;
	numSamples = 0;
	while( (timeSinceLastSpace_us < 1000) && (numSamples < maxSamples) ){
		if(GPIO_INPUT_GET(GPIO_ID_PIN(IR_PIN_NUM))){

			// this is the first time seeing a mark after recording a space
			if(timeSinceLastMark_us){
				// ok, so the pin went low, we should add this mark to the array
				samples[numSamples].time_us = timeSinceLastMark_us;
				samples[numSamples++].type = e_typeSpace;
			}

			timeSinceLastMark_us = 0;
			++timeSinceLastSpace_us;
		}
		else{
			// This is the first time seeing a space after recording a mark
			if(timeSinceLastSpace_us){
				// ok, so the pin went low, we should add this mark to the array
				samples[numSamples].time_us = timeSinceLastSpace_us;
				samples[numSamples++].type = e_typeMark;
			}
			timeSinceLastSpace_us = 0;
			++timeSinceLastMark_us;
		}
		os_delay_us(1);
	}
}

static void ICACHE_FLASH_ATTR dumpResults(void){
	int i = 0;
	os_printf("===DUMPING CAPTURE DATA===\n");
	for(i = 0; i < numSamples; ++i){
		os_printf("%d: %c%u\n", i, samples[i].type?'s':'m', samples[i].time_us);
	}
}

static void sendData(unsigned* delays, unsigned numDelays){
	const int numSamples = 34;
	int i = 0;
	int numTx = 0;
	int pulse, numPulses = 0;

	for(i = 0; i < numDelays; ++i){
		// mark (1)
		if( (i%2) == 0 ){
			// pulse width of 26us observed on logic analyzer
			numPulses = delays[i]/26;
			for(pulse= 0; pulse < numPulses; ++pulse){
				GPIO_OUTPUT_SET(GPIO_ID_PIN(TX_PIN_NUM), 1);
				os_delay_us(9);
				GPIO_OUTPUT_SET(GPIO_ID_PIN(TX_PIN_NUM), 0);
				os_delay_us(16);
			}
		}
		// space (0)
		else{
			//os_printf("space\n");
			GPIO_OUTPUT_SET(GPIO_ID_PIN(TX_PIN_NUM), 0);
			os_delay_us(delays[i]);
		}
		//GPIO_OUTPUT_SET(GPIO_ID_PIN(TX_PIN_NUM), 0);
		//os_delay_us(29710);
	}
	
	// we should leave it as 0
	GPIO_OUTPUT_SET(GPIO_ID_PIN(TX_PIN_NUM), 0);
}

static void ICACHE_FLASH_ATTR irISR(void* ignored){
		// Get the register that will tell us who triggered the interrupt
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

	os_printf("ISR: gpio_status = 0x%08X\n", gpio_status);
	// disable interrupt
	gpio_pin_intr_state_set(GPIO_ID_PIN(IR_PIN_NUM), GPIO_PIN_INTR_DISABLE);
		// clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(IR_PIN_NUM));

	if(GPIO_INPUT_GET(GPIO_ID_PIN(IR_PIN_NUM)) == 1){
		os_printf("Got positive edge interrupt\n");
	}
	else{
		os_printf("Negative edge interrupt\n");
		//collectData();
		//dumpResults();
	}

	os_printf("Leaving isr\n");
	// Restore interrupt
	gpio_pin_intr_state_set(GPIO_ID_PIN(IR_PIN_NUM), GPIO_PIN_INTR_NEGEDGE);
}

void sendFanPwrSpeedToggleCmd(void){
	unsigned delays[NUM_BITS_SPEED_TOGGLE_CMD] = {
	713,
	1789,
	713,
	1789,
	1978,
	525,
	1978,
	525,
	713,
	1789,
	713,
	1789,
	713,
	1789,
	1978,
	525,
	713,
	1789,
	1978,
	525,
	1978,
	525,
	713,
	1789,
	713,
	1789,
	713,
	1789,
	1978,
	525,
	713,
	1789,
	713,
	1789,
	1978,
	525,
	1978,
	525,
	713,
	1789,
	713,
	1789,
	713,
	1789,
	1978,
	525,
	713
	};

	sendData(delays, NUM_BITS_SPEED_TOGGLE_CMD);
}


bool ICACHE_FLASH_ATTR initFanControl(void){
	bool ret = false;
	// Setup interrupts. I guess this is a global setting?
	ETS_GPIO_INTR_ATTACH(irISR, NULL);
	ETS_GPIO_INTR_DISABLE();

	ret = setPinAsGpio(IR_PIN_NUM);
	ret = setPinAsGpio(TX_PIN_NUM);

	GPIO_OUTPUT_SET(GPIO_ID_PIN(IR_PIN_NUM), 1);

	// clear register of all sources?
    gpio_register_set(GPIO_PIN_ADDR(IR_PIN_NUM), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                      | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                      | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

	// configure irq for our pin num
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(IR_PIN_NUM));

	// enable interrupts. Start with listening on the negative edge
	gpio_pin_intr_state_set(GPIO_ID_PIN(IR_PIN_NUM), GPIO_PIN_INTR_NEGEDGE);

	// global interrupt enable?
	ETS_GPIO_INTR_ENABLE();


	GPIO_OUTPUT_SET(GPIO_ID_PIN(TX_PIN_NUM), 0);

	return ret;
}



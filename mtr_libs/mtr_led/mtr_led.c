#include "mtr_led/mtr_led.h"

//--------------------------------------------------------------
// Definition of LEDs
//--------------------------------------------------------------
MTR_Led_t LED[CONFIG_LED_MAX_AMOUNT];

errorType MTR_Led_init(MTR_Led_Name_t led, MTR_Port_Name_t portName, MTR_Port_Pin_t portPin){

    if (led >= CONFIG_LED_MAX_AMOUNT){
    	return E_INVALID_VALUE;
    }

    if (LED[led].inited == true){
    	return E_BUSY;
    }

	LED[led].ledPin = portPin;
	LED[led].ledPort = portName;
	LED[led].ledStatus = LED_OFF;
	LED[led].inited = true;

	MTR_Port_init(LED[led].ledPort, LED[led].ledPin, DIGITAL, PORT, OUT, SLOW, false, true);
	// Default state
	MTR_Led_off(led);
	return E_NO_ERROR;
}


void MTR_Led_on(MTR_Led_Name_t led)
{
	if (LED[led].inited == true)
	{
		LED[led].ledStatus = LED_ON;
		MTR_Port_setPin(LED[led].ledPort, LED[led].ledPin);
	}
}


void MTR_Led_off(MTR_Led_Name_t led)
{
	if (LED[led].inited == true)
	{
		LED[led].ledStatus = LED_OFF;
		MTR_Port_resetPin(LED[led].ledPort, LED[led].ledPin);
	}
}


void MTR_Led_switch(MTR_Led_Name_t led)
{
	if (LED[led].inited == true)
	{
		if(LED[led].ledStatus==LED_OFF){
			MTR_Led_on(led);
		}
		else {
			MTR_Led_off(led);
		}
	}
}


uint8_t MTR_Led_getStatus(MTR_Led_Name_t led){

	return LED[led].ledStatus;
}

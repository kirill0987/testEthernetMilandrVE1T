#include "mtr_button/mtr_button.h"

//--------------------------------------------------------------
// Definition of some buttons
//--------------------------------------------------------------
MTR_Button_t Buttons[CONFIG_BUTTON_MAX_AMOUNT] = {};

uint8_t currentButtonAmount = 0;


errorType MTR_Button_init(MTR_Button_Name_t button, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Button_Type_t type){

    bool_t butExist = false;
    uint8_t numberOfButtonInButtons = -1;

    for (uint8_t i = 0; i < currentButtonAmount; i++){
        if(Buttons[i].buttonName == button){
        	numberOfButtonInButtons = i;
            break;
        }
    }

    if (numberOfButtonInButtons != -1 && currentButtonAmount < CONFIG_BUTTON_MAX_AMOUNT){
    	numberOfButtonInButtons = currentButtonAmount;
    	currentButtonAmount += 1;
    }
    else
    {
    	return E_INVALID_VALUE;
    }

	Buttons[numberOfButtonInButtons].buttonName = button;
	Buttons[numberOfButtonInButtons].buttonPort = port;
	Buttons[numberOfButtonInButtons].buttonPin = pin;
	Buttons[numberOfButtonInButtons].type = type;
	Buttons[numberOfButtonInButtons].buttonStatus = 0;

	if (type == NormalButton){
		MTR_Port_init(port, pin, DIGITAL, PORT, IN, NORMAL, false, true);
	}
	else if (type == ReverseButton){
		MTR_Port_init(port, pin, DIGITAL, PORT, IN, NORMAL, true, false);
	}

    return E_NO_ERROR;
}


bool_t const * MTR_Button_read(MTR_Button_Name_t button)
{
    for (uint8_t i = 0; i < currentButtonAmount; i++){
        if(Buttons[i].buttonName == button){
        	return &Buttons[i].buttonStatus;
        }
    }

    return 0;
}


void MTR_Button_update(){
    uint32_t pressedCode = 0;
    uint32_t releasedCode = 0;
    uint32_t holdedCode = 0;
    uint32_t states = 0;
    bool_t holding_flag = false;

    //FOR EVERY INITED Buttons
    for (uint8_t i = 0; i < currentButtonAmount; i++){

		//READ CURRENT STATUS, PRESSED = true, NOT PRESSED = false
		bool_t currentStatus = MTR_Port_getBit(Buttons[i].buttonPort, Buttons[i].buttonPin);

		if (Buttons[i].type == ReverseButton){
			currentStatus = !currentStatus;
		}

		states |= currentStatus << Buttons[i].buttonName;

		if (currentStatus != Buttons[i].buttonStatus){
		    if (Buttons[i].buttonValue < CONFIG_BUTTON_TRIGGER_VALUE * 2){
		        Buttons[i].buttonValue += 2;
		    }
		}
		else {
		    Buttons[i].buttonValue = Buttons[i].buttonValue ? Buttons[i].buttonValue - 1 : 0;
		}

		if (currentStatus != Buttons[i].buttonStatus && Buttons[i].buttonValue >= CONFIG_BUTTON_TRIGGER_VALUE * 2){

		    if (currentStatus)
		    {
		        Buttons[i].buttonStatus = true;
	            Buttons[i].buttonValue = 0;
	            Buttons[i].holdingValue = 0;
	            pressedCode |= (1 << Buttons[i].buttonName);
		    }
		    else
		    {
	            Buttons[i].buttonStatus = false;
	            if (Buttons[i].butonHolding){
	            	Buttons[i].butonHolding = false;
	            }

#ifndef CONFIG_BUTTON_TRIGGER_RELEASE_AFTER_HOLDING
	            else{
	            	releasedCode |= (1 << Buttons[i].buttonName);
	            }
#else
	            releasedCode |= (1 << Buttons[i].buttonName);
#endif

		    }
		}
		if (Buttons[i].buttonStatus){
			Buttons[i].holdingValue++;
			if (Buttons[i].holdingValue >= CONFIG_BUTTON_HOLDING_VALUE){
				Buttons[i].butonHolding = true;
				Buttons[i].holdingValue = 0;
				holding_flag = true;
			}
			if (Buttons[i].butonHolding){
				holdedCode |= (1 << Buttons[i].buttonName);
			}
		}
    }

    MTR_Button_buttonStates(states);

    if (holding_flag){
    	MTR_Button_buttonHolded(holdedCode);
    }

    if (pressedCode){
        MTR_Button_buttonPressed(pressedCode);
    }

    if (releasedCode){
        MTR_Button_buttonReleased(releasedCode);
    }

}

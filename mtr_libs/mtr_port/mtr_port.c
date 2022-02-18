#include "mtr_port/mtr_port.h"
#include "MDR32F9Qx_rst_clk.h"

MTR_Port_t PORTS[6] = {
	{PORTA, MDR_PORTA, RST_CLK_PCLK_PORTA},
	{PORTB, MDR_PORTB, RST_CLK_PCLK_PORTB},
	{PORTC, MDR_PORTC, RST_CLK_PCLK_PORTC},
	{PORTD, MDR_PORTD, RST_CLK_PCLK_PORTD},
	{PORTE, MDR_PORTE, RST_CLK_PCLK_PORTE},
	{PORTF, MDR_PORTF, RST_CLK_PCLK_PORTF}
};

MTR_Port_ConnectedPins_t connectedPins;

/***** Functions *****/

void MTR_Port_deInit(MTR_Port_Name_t port){

    PORT_DeInit(PORTS[port].portOriginName);
}

void MTR_Port_init(	MTR_Port_Name_t port,
					MTR_Port_Pin_t pin,
					MTR_Port_Mode_t mode,
					MTR_Port_Func_t func,
					MTR_Port_Direction_t direction,
					MTR_Port_Speed_t speed,
					status_t pullUp,
					status_t pullDown)
{

    PORT_InitTypeDef portInitStructure;

    RST_CLK_PCLKcmd(PORTS[port].RST_CLK_PCLK, ENABLE);

    portInitStructure.PORT_FUNC = func;
    portInitStructure.PORT_GFEN = PORT_GFEN_OFF;
    portInitStructure.PORT_MODE = mode;
    portInitStructure.PORT_OE = direction;
    portInitStructure.PORT_PD = PORT_PD_DRIVER;
    portInitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
    portInitStructure.PORT_Pin = pin;
    portInitStructure.PORT_PULL_DOWN = pullDown ? PORT_PULL_DOWN_ON : PORT_PULL_DOWN_OFF;
    portInitStructure.PORT_PULL_UP = pullUp ? PORT_PULL_UP_ON : PORT_PULL_UP_OFF;
    portInitStructure.PORT_SPEED = speed;

    PORT_Init(PORTS[port].portOriginName, &portInitStructure);
    PORTS[port].portOriginName->RXTX &= ~pin;

}

void MTR_Port_initAnalog(MTR_Port_Name_t port, MTR_Port_Pin_t pin){

    PORT_InitTypeDef portInitStructure;

    RST_CLK_PCLKcmd(PORTS[port].RST_CLK_PCLK, ENABLE);

    portInitStructure.PORT_FUNC = PORT_FUNC_PORT;
    portInitStructure.PORT_GFEN = PORT_GFEN_OFF;
    portInitStructure.PORT_MODE = PORT_MODE_ANALOG;
    portInitStructure.PORT_OE = IN;
    portInitStructure.PORT_PD = PORT_PD_DRIVER;
    portInitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
    portInitStructure.PORT_Pin = pin;
    portInitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
    portInitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
    portInitStructure.PORT_SPEED = PORT_SPEED_SLOW;

    PORT_Init(PORTS[port].portOriginName, &portInitStructure);

}

void MTR_Port_initDigital(MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Direction_t direction){

    PORT_InitTypeDef portInitStructure;

    RST_CLK_PCLKcmd(PORTS[port].RST_CLK_PCLK, ENABLE);

    portInitStructure.PORT_FUNC = PORT_FUNC_PORT;
    portInitStructure.PORT_GFEN = PORT_GFEN_OFF;
    portInitStructure.PORT_MODE = PORT_MODE_DIGITAL;
    portInitStructure.PORT_OE = direction;
    portInitStructure.PORT_PD = PORT_PD_DRIVER;
    portInitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
    portInitStructure.PORT_Pin = pin;
    portInitStructure.PORT_PULL_DOWN = direction == IN ? PORT_PULL_DOWN_ON : PORT_PULL_DOWN_OFF;
    portInitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
    portInitStructure.PORT_SPEED = PORT_SPEED_SLOW;

    PORTS[port].portOriginName->RXTX &= ~pin;
    PORT_Init(PORTS[port].portOriginName, &portInitStructure);

}

void MTR_Port_initWithStruct(MTR_Port_SomePin_t initUartSruct){

	PORT_InitTypeDef portInitStructure;

    RST_CLK_PCLKcmd(PORTS[initUartSruct.portName].RST_CLK_PCLK, ENABLE);

    portInitStructure.PORT_FUNC = initUartSruct.func;
    portInitStructure.PORT_GFEN = PORT_GFEN_OFF;
    portInitStructure.PORT_MODE = initUartSruct.mode;
    portInitStructure.PORT_OE = initUartSruct.direction;
    portInitStructure.PORT_PD = PORT_PD_DRIVER;
    portInitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
    portInitStructure.PORT_Pin = initUartSruct.portPin;
    portInitStructure.PORT_PULL_DOWN =  initUartSruct.pullDown ? PORT_PULL_DOWN_ON : PORT_PULL_DOWN_OFF;
    portInitStructure.PORT_PULL_UP = initUartSruct.pullUp ? PORT_PULL_UP_ON : PORT_PULL_UP_OFF;;
    portInitStructure.PORT_SPEED = initUartSruct.speed;

    PORT_Init(PORTS[initUartSruct.portName].portOriginName, &portInitStructure);
}

void MTR_Port_setPin(MTR_Port_Name_t port, MTR_Port_Pin_t pin){

    PORT_SetBits(PORTS[port].portOriginName, pin);
}

void MTR_Port_resetPin(MTR_Port_Name_t port, MTR_Port_Pin_t pin){

    PORT_ResetBits(PORTS[port].portOriginName, pin);
}

bool_t MTR_Port_getBit(MTR_Port_Name_t port, MTR_Port_Pin_t pin){

	return (bool_t) PORT_ReadInputDataBit(PORTS[port].portOriginName, pin);

}


uint16_t MTR_Port_getPort(MTR_Port_Name_t port){

	return PORT_ReadInputData(PORTS[port].portOriginName);

}


void MTR_Port_writeBit(MTR_Port_Name_t port, MTR_Port_Pin_t pin, bitStatus_t status){

	PORT_WriteBit(PORTS[port].portOriginName, pin, status);
}


MDR_PORT_TypeDef* MTR_Port_getOriginStruct(MTR_Port_Name_t port){

	return PORTS[port].portOriginName;
}


void MTR_Port_connectPin(MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Direction_t direction, bool_t* value){

	if (connectedPins.currentPinAmount < CONFIG_PORT_CONNECTED_PIN_AMOUNT){

		connectedPins.pins[connectedPins.currentPinAmount].portName = port;
		connectedPins.pins[connectedPins.currentPinAmount].portPin = pin;
		connectedPins.pins[connectedPins.currentPinAmount].direction = direction;
		connectedPins.pins[connectedPins.currentPinAmount].connectedWith = value;

		connectedPins.currentPinAmount += 1;

		MTR_Port_init(port, pin, DIGITAL, PORT, direction, SLOW, disable, enable);

		if (direction == OUT){
			MTR_Port_resetPin(port, pin);
		}
	}
}


void MTR_Port_updatePins(){

	MTR_Port_ConnectedPin_t* currentPin;
	bool_t incomeDataChanged = false;

	for (uint8_t i = 0; i < connectedPins.currentPinAmount; i++){
		currentPin = &connectedPins.pins[i];
		if (currentPin->direction == OUT)
		{
			if (*currentPin->connectedWith){
				MTR_Port_setPin(currentPin->portName, currentPin->portPin);
			}
			else {
				MTR_Port_resetPin(currentPin->portName, currentPin->portPin);
			}
		}

		if (currentPin->direction == IN)
		{
			bool_t newState = MTR_Port_getBit(currentPin->portName, currentPin->portPin);
			bool_t prevState = *currentPin->connectedWith;

			*currentPin->connectedWith = newState;

			if (prevState != newState)
			{
				MTR_Port_pinStateChanged(*currentPin);
				incomeDataChanged = true;
			}
		}
	}

	if (incomeDataChanged){
		MTR_Port_incomePinStatesDataChanged();
	}
}

void MTR_Port_getPinFromUint32(uint32_t value, MTR_Port_SomePin_t* pinToWrite){

	pinToWrite->portPin = value & 0xFFFF;
	pinToWrite->portName = 0xF &(value >> 16);
	pinToWrite->func =  0x3 & (value  >>  20);
	pinToWrite->mode = getBit(value, 22);
	pinToWrite->direction = getBit(value, 23);
	pinToWrite->pullDown = getBit(value, 24);
	pinToWrite->pullUp = getBit(value, 25);
	pinToWrite->speed = 0x3 & (value >> 26);
}

uint32_t MTR_Port_getUint32FromPin(MTR_Port_SomePin_t* pin){

	uint32_t temp = 0;

	temp = pin->portPin & 0xFFFF;
	temp |= ((pin->portName & 0xF) << 16);
	temp |= ((pin->func & 0x3) << 20);
	temp |= ((pin->mode & 0x1) << 22);
	temp |= ((pin->direction & 0x1) << 23);
	temp |= ((pin->pullDown  & 0x1) << 24);
	temp |= ((pin->pullUp & 0x1) << 25);
	temp |= ((pin->speed & 0x3) << 26);

	return temp;
}

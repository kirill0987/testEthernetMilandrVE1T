#include "mtr_uart_base/mtr_uart_base.h"
#include "mtr_eeprom/mtr_eeprom.h"
#include "MDR32F9Qx_rst_clk.h"
#include <string.h>
#include <malloc.h>

/**
 * @brief Add byte to RX buffer for reading.
 * @param uart : The name of uart, can be UART1 or UART2;
 * @param byte : byte to read;
 * @return error
 *
 */
errorType MTR_Uart_addByteToRxBuffer(AvaliableConnectionType_t uart, uint8_t byte);

/**
 * @brief Init the uart timer for handlers
 * @return void
 */
static void MTR_Uart_timerInit(AvaliableConnectionType_t uart, uint32_t baudSpeed);

static volatile Uart_CommunicationStruct_t* uartCommunicationDataMainStructs[UART_AMOUNT];
static uint32_t currentTimerBaudSpeed;
UART_InitTypeDef uartInitStructure;
MTR_Uart_CanRxMsg_t rxCanSerialMessage;
MDR_TIMER_TypeDef * timer;

static volatile Uart_SettingsStruct_t uartSettingsMainStructs[UART_AMOUNT] = {
  // Name,  UART,	     Clock,				  Baud,   Interrupt, Type,
  {MDR_UART1, RST_CLK_PCLK_UART1, 115200, UART_WordLength8b, UART_StopBits1, UART_Parity_No, UART1_IRQn, UART, false,
  //RS-485 switch pin
  {PORTA, PIN0, ANALOG, ALTER, IN, SLOW, disable, enable},
  //PORT, PIN, Clock
  {PORTB, PIN5, DIGITAL, ALTER, OUT, MAX, disable, disable},  // TX
  {PORTB, PIN6, DIGITAL, ALTER, IN, MAX, disable, disable}, false}, // RX

// Name,	 UART,     Clock,			  AF-UART,			Baud,	Interrupt, Type
  {MDR_UART2, RST_CLK_PCLK_UART2, 115200, UART_WordLength8b, UART_StopBits1, UART_Parity_No, UART2_IRQn, UART, false,
//RS-485 switch pin
  {PORTB, PIN0, DIGITAL, PORT, OUT, MAX, disable, enable},
// PORT ,	 PIN      , Clock
  {PORTF, PIN1, DIGITAL, OVERRID, OUT, MAX, disable, disable},  // TX
  {PORTF, PIN0, DIGITAL, OVERRID, IN, MAX, disable, disable}, false},  // RX
};

void MTR_Uart_init(AvaliableConnectionType_t uart)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited == false)
	{
		uartCommunicationDataMainStructs[uart - baseUartValue] = calloc(sizeof(Uart_CommunicationStruct_t), 1);
		uartCommunication = MTR_Uart_getCommunicationStruct(uart);
		uartSettings->inited = true;
	}

    RST_CLK_PCLKcmd(uartSettings->rst_clk, ENABLE);

    UART_DeInit(uartSettings->origin);

    MTR_Uart_clearITbit(uart, UART_IT_TX);
    MTR_Uart_clearITbit(uart, UART_IT_RX);

    MTR_Uart_initGPIO(uart);
    MTR_Uart_setDirection(uart, IN);

    UART_BRGInit(uartSettings->origin, CONFIG_UART_HLCK);

    //UART configuration
    uartInitStructure.UART_BaudRate = uartSettings->baud;
    uartInitStructure.UART_WordLength = uartSettings->dataLength;
    uartInitStructure.UART_StopBits = uartSettings->stopBites;
    uartInitStructure.UART_Parity = uartSettings->parity;
    uartInitStructure.UART_FIFOMode = UART_FIFO_OFF;
    uartInitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

    //Uart init
    UART_Init (uartSettings->origin, &uartInitStructure);

    //find max uart baud speed
    uint32_t baudSpeedToInit = 0;
    for (uint8_t i = 0; i < UART_AMOUNT; i++){
    	if (uartSettingsMainStructs[i].inited){
			baudSpeedToInit = (baudSpeedToInit < uartSettingsMainStructs[i].baud) ? uartSettingsMainStructs[i].baud : baudSpeedToInit;
    	}
    }

    for (uint8_t i = 0; i < UART_AMOUNT; i++){
		if (uartSettingsMainStructs[i].inited)
		{
			uartCommunicationDataMainStructs[i]->numsOfRepeats[uartTimerRepeatesSwitchToReadAfterTransmit] = (baudSpeedToInit / uartSettingsMainStructs[i].baud) * 2;
			uartCommunicationDataMainStructs[i]->numsOfRepeats[uartTimerRepeatesProccessingRecievedMessage] = (CONFIG_UART_PROCESSING_RECIEVED_MESSAGE_IN_BYTES) * (baudSpeedToInit / uartSettingsMainStructs[i].baud);
			uartCommunicationDataMainStructs[i]->numsOfRepeats[uartTimerRepeatesStartTransmit] = (baudSpeedToInit / uartSettingsMainStructs[i].baud);
		}
    }

	MTR_Timer_deInit(CONFIG_UART_TIMER);
	MTR_Uart_timerInit(uart, baudSpeedToInit);

	//Clear buffers
	MTR_Uart_resetRxPointer(uart, uartCommunication->rxBuffer);
	MTR_Uart_resetRxPointer(uart, uartCommunication->rxBuffer + 1);

    MTR_Uart_resetTxPointer(uart);
}

void MTR_Uart_initGPIO(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

    MTR_Port_initWithStruct(uartSettings->rxPin);
    MTR_Port_initWithStruct(uartSettings->txPin);

    if (uartSettings->type == RS485) {
    	MTR_Port_initWithStruct(uartSettings->enPin);
    }
}

void MTR_Uart_start(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];
	UART_Cmd(uartSettings->origin, ENABLE);
}

void MTR_Uart_stop(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];
	UART_Cmd(uartSettings->origin, DISABLE);
}

void MTR_Uart_deInit(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];
	UART_DeInit(uartSettings->origin);
}

Uart_SettingsStruct_t* MTR_Uart_getSettingsStruct(AvaliableConnectionType_t uart)
{
	switch (uart) {
		case Uart2Connection:
			return &uartSettingsMainStructs[uart - baseUartValue];
			break;

		default:
			return uartSettingsMainStructs;
			break;
	}
}

MTR_Uart_Type_t MTR_Uart_getType(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	return uartSettings->type;
}

void MTR_Uart_setType(AvaliableConnectionType_t uart, MTR_Uart_Type_t newType)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];
	uartSettings->type = newType;
}

uint32_t MTR_Uart_getSpeed(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];
	return uartSettings->baud;
}

void MTR_Uart_setSpeed(AvaliableConnectionType_t uart, uint32_t baud)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (baud) {
		case 9600:
			uartSettings->baud = 9600;
			break;
		case 19200:
			uartSettings->baud = 19200;
			break;
		case 38400:
			uartSettings->baud = 38400;
			break;
		case 57600:
			uartSettings->baud  = 57600;
			break;
		case 230400:
			uartSettings->baud = 230400;
			break;
		case 460800:
			uartSettings->baud = 460800;
			break;
		case 921600:
			uartSettings->baud = 921600;
			break;
		default:
			uartSettings->baud = 115200;
			break;
	}
}

uint8_t MTR_Uart_getDataLength(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (uartSettings->dataLength) {
		case UART_WordLength5b:
			return 5;
		case UART_WordLength6b:
			return 6;
		case UART_WordLength7b:
			return 7;
		case UART_WordLength8b:
			return 8;
	}
	return 0;
}

void MTR_Uart_setDataLength(AvaliableConnectionType_t uart, uint8_t dataLength)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (dataLength) {
	case 5:
		uartSettings->dataLength = UART_WordLength5b;
		break;
	case 6:
		uartSettings->dataLength = UART_WordLength6b;
		break;
	case 7:
		uartSettings->dataLength = UART_WordLength7b;
		break;
	default:
		uartSettings->dataLength = UART_WordLength8b;
		break;
	}
}

uint8_t MTR_Uart_getStopBites(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (uartSettings->stopBites) {
		case UART_StopBits1:
			return 1;
		case UART_StopBits2:
			return 2;
		default:
			return 0;
	}
}

void MTR_Uart_setStopBites(AvaliableConnectionType_t uart, uint8_t stopBites)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (stopBites) {
	case 2:
		uartSettings->stopBites = UART_StopBits2;
		break;
	default :
		uartSettings->stopBites = UART_StopBits1;
	}
}

uint8_t MTR_Uart_getParity(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (uartSettings->parity) {
		case UART_Parity_No:
			return 0;
		case UART_Parity_Odd:
			return 1;
		case UART_Parity_Even:
			return 2;
	}

	return 0xFF;
}

void MTR_Uart_setParity(AvaliableConnectionType_t uart, uint8_t parity)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	switch (parity){
	case 0:
		uartSettings->parity = UART_Parity_No;
		break;
	case 1:
		uartSettings->parity = UART_Parity_Odd;
		break;
	case 2:
		uartSettings->parity = UART_Parity_Even;
		break;
	default :
		uartSettings->parity = UART_Parity_No;
	}
}

uint8_t MTR_Uart_readByte(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];
	return UART_ReceiveData(uartSettings->origin);
}

void MTR_Uart_addByte(AvaliableConnectionType_t uart, uint8_t byte)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited){

		if (uartCommunication->txBuffer.b.amount < CONFIG_UART_BUFFER_BYTE_SIZE){
			uartCommunication->txBuffer.b.buffer[uartCommunication->txBuffer.b.amount]= byte;

			uartCommunication->txBuffer.b.amount++;
			//readpos setting later
		}
	}
}

void MTR_Uart_addHalfWord(AvaliableConnectionType_t uart, uint16_t halfWord)
{
	MTR_Uart_addByte(uart, (uint8_t) (halfWord >> 8));
	MTR_Uart_addByte(uart, (uint8_t) halfWord);
}

void MTR_Uart_addWord(AvaliableConnectionType_t uart, uint32_t word)
{
	MTR_Uart_addHalfWord(uart, (uint16_t) (word >> 16));
	MTR_Uart_addHalfWord(uart, (uint16_t) word);
}

errorType MTR_Uart_addString(AvaliableConnectionType_t uart, char* pointer, MTR_Uart_Lastbyte_t end_cmd)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited)
	{
		if (*pointer != 0 ) {
			while (*pointer != 0){
				//copy to buffer 1 symbol at once
				MTR_Uart_addByte(uart, *pointer);
				pointer += 1;
			}
		}
		else {
			return E_NULL_POINTER;
		}

		switch (end_cmd) {

			case LFCR:
				MTR_Uart_addByte(uart, 0x0A);
				MTR_Uart_addByte(uart, 0x0D);
				break;

			case CRLF:
				MTR_Uart_addByte(uart, 0x0D);
				MTR_Uart_addByte(uart, 0x0A);
				break;

			case LF:
				MTR_Uart_addByte(uart, 0x0A);
				break;

			case CR:
				MTR_Uart_addByte(uart, 0x0D);
				break;

			case COLON:
				MTR_Uart_addByte(uart, 0x3A);
				break;

			case SP:
				MTR_Uart_addByte(uart, 0x20);
				break;

			case DOT:
				MTR_Uart_addByte(uart, 0x2E);
				break;

			default:
				break;
		}

		return E_NO_ERROR;
	}

	return E_STATE;
}

errorType MTR_Uart_addArray(AvaliableConnectionType_t uart, uint8_t const* data, uint16_t cnt)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited){

		if (cnt == 0 || data == 0){
			return E_NULL_POINTER;
		}

		for(uint16_t n = 0; n < cnt; n++) {
			MTR_Uart_addByte(uart, *(data + n));
		}

		return E_NO_ERROR;

	}

	return E_STATE;
}

void MTR_Uart_addCrc16(AvaliableConnectionType_t uart, uint16_t(*crc16Func)(uint8_t*, uint32_t))
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited)
	{
		if (uartCommunication->txBuffer.b.amount + 2 <= CONFIG_UART_BUFFER_BYTE_SIZE)
		{
			uint16_t crc = crc16Func((uint8_t*) &uartCommunication->txBuffer.b.buffer, (uint32_t) uartCommunication->txBuffer.b.amount);
			MTR_Uart_addHalfWord(uart, crc);
		}
	}
}

errorType MTR_Uart_addByteToRxBuffer(AvaliableConnectionType_t uart, uint8_t byte)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited)
	{
		if (uartCommunication->currentReceiveBuffer == 0){
			//need select buffer
			if (uartCommunication->rxBuffer[0].free){
				uartCommunication->currentReceiveBuffer = &uartCommunication->rxBuffer[0];
				uartCommunication->currentReceiveBuffer->free = false;
			}
			else {
				uartCommunication->currentReceiveBuffer = &uartCommunication->rxBuffer[1];
			}
		}

		uartCommunication->totalBytesRecieved += 1;

		MTR_Uart_BufferStruct_t* buffer = uartCommunication->currentReceiveBuffer;

		if (buffer->amount < CONFIG_UART_BUFFER_BYTE_SIZE)
		{
			buffer->buffer[buffer->amount] = byte;
			buffer->amount++;

			return E_NO_ERROR;
		}
		else {
			return E_OUT_OF_RANGE;
		}
	}
	return E_STATE;
}

void MTR_Uart_resetRxPointer(AvaliableConnectionType_t uart, MTR_Uart_BufferStruct_t* buffer) {
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	if (buffer){
		MTR_Uart_resetBuffer(buffer);
	}
	else {
		//reset first default buffer
		MTR_Uart_resetBuffer(&uartCommunication->rxBuffer[0]);
	}
}

void MTR_Uart_resetBuffer(MTR_Uart_BufferStruct_t* buffer) {
	buffer->free = true;
	buffer->amount = 0;
}

void MTR_Uart_resetTxPointer(AvaliableConnectionType_t uart)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited)
	{
		uartCommunication->txBuffer.b.amount = 0;
		uartCommunication->txBuffer.readPos = 0;
	}
}

void MTR_Uart_setDirection(AvaliableConnectionType_t uart, MTR_Port_Direction_t direction)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (direction == IN )
	{
		uartCommunication->direction = IN;

		uartSettings->origin->CR &= ~(1<<8);
		uartSettings->origin->CR |=  (1<<9);

		if (uartSettings->type == RS485)
		{
			if (uartSettings->enableEnablePinInvesion) {
				MTR_Port_setPin(uartSettings->enPin.portName, uartSettings->enPin.portPin);
			}
			else {
				MTR_Port_resetPin(uartSettings->enPin.portName, uartSettings->enPin.portPin);
			}
		}
	}
	else
	{
		uartCommunication->direction = OUT;

		uartSettings->origin->CR &= ~(1<<9);
		uartSettings->origin->CR |=  (1<<8);

		if (uartSettings->type == RS485)
		{
			if (uartSettings->enableEnablePinInvesion){
				MTR_Port_resetPin(uartSettings->enPin.portName, uartSettings->enPin.portPin);
			}
			else {
				MTR_Port_setPin(uartSettings->enPin.portName, uartSettings->enPin.portPin);
			}
		}
	}
}

MTR_Port_Direction_t MTR_Uart_getDirection(AvaliableConnectionType_t uart)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited){
		return uartCommunication->direction;
	}

	return IN;
}

errorType MTR_Uart_initTransmite(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited)
	{
		if (MTR_Uart_getDirection(uart) == IN)
		{
			MTR_Uart_setDirection(uart, OUT);
			MTR_Uart_timerRefresh(uart, uartTimerRepeatesStartTransmit);
			return E_NO_ERROR;
		}
		else {
			return E_BUSY;
		}
	}
	return E_STATE;
}

void MTR_Uart_sendNextByte(AvaliableConnectionType_t uart)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	uint16_t* read_pos = &(uartCommunication->txBuffer.readPos);

	uartCommunication->totalBytesSended++;
	uartCommunication->txBuffer.b.amount -= 1;

	if (*read_pos >= (CONFIG_UART_BUFFER_BYTE_SIZE - 1)) {
		*read_pos=0;
	}
	else {
		*read_pos += 1;
	}

	uartCommunication->lastSendedByte = uartCommunication->txBuffer.b.buffer[(*read_pos - 1)];
	UART_SendData(uartSettings->origin, uartCommunication->txBuffer.b.buffer[(*read_pos - 1)]);
}

void MTR_Uart_enableInterrupt(AvaliableConnectionType_t uart, uint32_t uart_it)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

    UART_ClearITPendingBit(uartSettings->origin, uart_it);
    UART_ITConfig (uartSettings->origin, uart_it, ENABLE);
}

void MTR_Uart_clearITbit(AvaliableConnectionType_t uart, uint32_t uart_it)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	UART_ClearITPendingBit(uartSettings->origin, uart_it);
}

void MTR_Uart_enableInterrupts(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

    UART_ITConfig (uartSettings->origin, UART_IT_RX, ENABLE);
    UART_ITConfig (uartSettings->origin, UART_IT_TX, ENABLE);
    UART_ITConfig (uartSettings->origin, UART_IT_OE, ENABLE);

	UART_ITConfig (uartSettings->origin, UART_IT_BE, ENABLE);
	UART_ITConfig (uartSettings->origin, UART_IT_PE, ENABLE);
	UART_ITConfig (uartSettings->origin, UART_IT_FE, ENABLE);

//	UART_ITConfig (UartStr->origin, UART_IT_DSR, ENABLE);
//	UART_ITConfig (UartStr->origin, UART_IT_DCD, ENABLE);
//	UART_ITConfig (UartStr->origin, UART_IT_CTS, ENABLE);
//	UART_ITConfig (UartStr->origin, UART_IT_RI, ENABLE);

}

void MTR_Uart_initNvic(AvaliableConnectionType_t uart)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

    NVIC_SetPriority(uartSettings->interrupt, CONFIG_UART_NVIC_PRIORITY);
	NVIC_EnableIRQ(uartSettings->interrupt);
}

void MTR_Uart_setTxPin(AvaliableConnectionType_t uart, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	uartSettings->txPin.func = func;
	uartSettings->txPin.portPin = pin;
	uartSettings->txPin.portName = port;
	uartSettings->txPin.direction = OUT;
	uartSettings->txPin.mode = DIGITAL;
	uartSettings->txPin.speed = MAX;
}

void MTR_Uart_setRxPin(AvaliableConnectionType_t uart, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	uartSettings->rxPin.func = func;
	uartSettings->rxPin.portPin = pin;
	uartSettings->rxPin.portName = port;
	uartSettings->rxPin.direction = IN;
	uartSettings->rxPin.mode = DIGITAL;
	uartSettings->rxPin.speed = MAX;
}

void MTR_Uart_setEnPin(AvaliableConnectionType_t uart, MTR_Port_Name_t port, MTR_Port_Pin_t pin, bool_t inversion)
{
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	uartSettings->enPin.func = PORT;
	uartSettings->enPin.portPin = pin;
	uartSettings->enPin.portName = port;
	uartSettings->enPin.direction = OUT;
	uartSettings->enPin.mode = DIGITAL;
	uartSettings->enPin.speed = MAX;
	uartSettings->enableEnablePinInvesion = inversion;
}

Uart_CommunicationStruct_t* MTR_Uart_getCommunicationStruct(AvaliableConnectionType_t uart)
{
	if (uart == Uart2Connection){
		return uartCommunicationDataMainStructs[1];
	}
	else {
		return uartCommunicationDataMainStructs[0];
	}
}

uint32_t MTR_Uart_getTimerBaudSpeed(){
	return currentTimerBaudSpeed;
}

void MTR_Uart_setNumOfRepeats(AvaliableConnectionType_t uart, uint16_t repeatNum, uint16_t value){

	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited)
	{
		if (repeatNum > 0 && repeatNum < UART_TIMER_NUMS_TOTAL_AMOUNT){
			uartCommunication->numsOfRepeats[repeatNum] = value;
		}
	}
}

uint16_t MTR_Uart_getTxBufferAmount(AvaliableConnectionType_t uart)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	return uartCommunication->txBuffer.b.amount;
}

errorType MTR_Uart_timerReset(AvaliableConnectionType_t uart, uint16_t timerNum)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];

	MTR_Timer_stop(CONFIG_UART_TIMER);

	if (timerNum < UART_TIMER_NUMS_TOTAL_AMOUNT){

		uartCommunication->needTimerNum = timerNum;
		uartCommunication->timerCounterDown = uartCommunication->numsOfRepeats[timerNum] + 1;
		MTR_Timer_setCounter(CONFIG_UART_TIMER, 0);

		return E_NO_ERROR;
	}

	return E_INVALID_VALUE;
}

void MTR_Uart_timerRefresh(AvaliableConnectionType_t uart, uint16_t timerNum)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];

	if (timerNum < UART_TIMER_NUMS_TOTAL_AMOUNT)
	{
		uartCommunication->needTimerNum = timerNum;
		uartCommunication->timerCounterDown = uartCommunication->numsOfRepeats[timerNum];
	}

	MTR_Timer_start(CONFIG_UART_TIMER);
}

void MTR_Uart_timerInit(AvaliableConnectionType_t uart, uint32_t baudSpeed)
{
	//time of transmit byte
	//MTR_Timer_initFreq(CONFIG_UART_TIMER, (baudSpeed / 8));

	MTR_Timer_init(CONFIG_UART_TIMER, 0, (921600 / baudSpeed), (CONFIG_CLOCK_TIMER_HZ / (921600 / 8)));

	MTR_Timer_setCountDirection(CONFIG_UART_TIMER, TIMER_CntDir_Up);

	MTR_Timer_initIT(CONFIG_UART_TIMER, TIMER_STATUS_CNT_ARR, enable);

	MTR_Timer_initNvic(CONFIG_UART_TIMER);

	MTR_Uart_timerReset(uart, 0);

	currentTimerBaudSpeed = baudSpeed;

	timer = MTR_Timer_getOriginStruct(CONFIG_UART_TIMER);
}

void MTR_Uart_callCallbacks(){

	for (uint8_t i = 0; i < UART_AMOUNT; i++){

		if (!uartSettingsMainStructs[i].inited) {
			continue;
		}

	    if (uartCommunicationDataMainStructs[i]->switchToRecieve){
	    	if (getBit(uartCommunicationDataMainStructs[i]->callCallbacksMask, uartTimerRepeatesSwitchToReadAfterTransmit)) {
	    		uartCommunicationDataMainStructs[i]->callCallbacksMask &= ~(1 << uartTimerRepeatesSwitchToReadAfterTransmit);
	    		uartCommunicationDataMainStructs[i]->switchToRecieve((AvaliableConnectionType_t)(Uart1Connection + i));
			}
		}

	    //max pririty if can
	    if (getBit(uartCommunicationDataMainStructs[i]->callCallbacksMask, uartTimerRepeatesProccessingRecievedMessage)){
	    	uartCommunicationDataMainStructs[i]->callCallbacksMask &= ~(1 << uartTimerRepeatesProccessingRecievedMessage);

	    	uartCommunicationDataMainStructs[i]->callbacksReceiveBuffer = uartCommunicationDataMainStructs[i]->currentReceiveBuffer;
	    	uartCommunicationDataMainStructs[i]->currentReceiveBuffer = 0;

#ifdef CONFIG_UART_SERIAL_CAN_ENABLE
			if (uartCommunicationDataMainStructs[i]->userCanRecieve){
				if (MTR_Uart_parseToCanMessage(&rxCanSerialMessage, uartCommunicationDataMainStructs[i]->callbacksRecieveBuffer, Uart1Connection + i)){
					uartCommunicationDataMainStructs[i]->callbacksRecieveBuffer->free = true;
					uartCommunicationDataMainStructs[i]->userCanRecieve((AvaliableConnectionType_t)(Uart1Connection + i), rxCanSerialMessage);

				}
			}
			else
#endif
			{
				if (uartCommunicationDataMainStructs[i]->userReceive){
					uartCommunicationDataMainStructs[i]->userReceive((AvaliableConnectionType_t)(Uart1Connection + i));
				}
			}

			//ATTENTION
			//User need to reset buffer by himself
			//If he not reset it, he will get second rx buffer every time
			//Second buffer have no check for free flag for library use

			//MTR_Uart_resetBuffer(uartCommunicationDataMainStructs[i]->callbacksRecieveBuffer);
			uartCommunicationDataMainStructs[i]->callbacksReceiveBuffer = 0;
		}


		if (uartCommunicationDataMainStructs[i]->userTimeout1){
			if (getBit(uartCommunicationDataMainStructs[i]->callCallbacksMask, uartTimerRepeatesUserTimeout1)){
				uartCommunicationDataMainStructs[i]->callCallbacksMask &= ~(1 << uartTimerRepeatesUserTimeout1);

				uartCommunicationDataMainStructs[i]->userTimeout1((AvaliableConnectionType_t)(Uart1Connection + i));
			}
		}

		if (uartCommunicationDataMainStructs[i]->userTimeout2){
			if (getBit(uartCommunicationDataMainStructs[i]->callCallbacksMask, uartTimerRepeatesUserTimeout2)){
				uartCommunicationDataMainStructs[i]->callCallbacksMask &= ~(1 << uartTimerRepeatesUserTimeout2);

				uartCommunicationDataMainStructs[i]->userTimeout2((AvaliableConnectionType_t)(Uart1Connection + i));
			}
		}

		if (uartCommunicationDataMainStructs[i]->userTimeout3){
			if (getBit(uartCommunicationDataMainStructs[i]->callCallbacksMask, uartTimerRepeatesUserTimeout3)){
				uartCommunicationDataMainStructs[i]->callCallbacksMask &= ~(1 << uartTimerRepeatesUserTimeout3);

				uartCommunicationDataMainStructs[i]->userTimeout3((AvaliableConnectionType_t)(Uart1Connection + i));
			}
		}

		if (uartCommunicationDataMainStructs[i]->userTimeout4){
			if (getBit(uartCommunicationDataMainStructs[i]->callCallbacksMask, uartTimerRepeatesUserTimeout4)){
				uartCommunicationDataMainStructs[i]->callCallbacksMask &= ~(1 << uartTimerRepeatesUserTimeout4);

				uartCommunicationDataMainStructs[i]->userTimeout4((AvaliableConnectionType_t)(Uart1Connection + i));
			}
		}
	}
}


void MTR_Uart_updateTimerNumsFromIRQ(AvaliableConnectionType_t uart)
{
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];
	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	if (uartSettings->inited && uartCommunication->timerCounterDown == 0)
	{
		switch (uartCommunication->needTimerNum){
			case uartTimerRepeatesStartTransmit:

				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;

				if (uartCommunication->txBuffer.b.amount){
					MTR_Uart_sendNextByte(uart);
				}

				break;

			case uartTimerRepeatesSwitchToReadAfterTransmit:
				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;
				uartCommunication->callCallbacksMask |= (1 << uartTimerRepeatesSwitchToReadAfterTransmit);

				MTR_Uart_setDirection(uart, IN);
				MTR_Uart_resetTxPointer(uart);

			break;

			case uartTimerRepeatesProccessingRecievedMessage:
				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;
				uartCommunication->callCallbacksMask |= (1 << uartTimerRepeatesProccessingRecievedMessage);

#ifdef CONFIG_UART_ENABLE_MAGIC_CODE
				if (uartCommunication->currentReceiveBuffer->amount == sizeof(toBootloaderResetCommand)){
					if (!memcmp(uartCommunication->currentReceiveBuffer, toBootloaderResetCommand, (uint32_t) sizeof(toBootloaderResetCommand)))
					{

			#ifdef USE_MDR1986VE9x
						__disable_fault_irq();
			#endif
						__disable_irq();

						MTR_Parameter_write(PARAMETER_BOOTSELECT, uart);
						MTR_Parameter_write(PARAMETER_CONNECTION_RXPINPORT, MTR_Port_getUint32FromPin(&uartSettings->rxPin));
						MTR_Parameter_write(PARAMETER_CONNECTION_TXPINPORT, MTR_Port_getUint32FromPin(&uartSettings->txPin));
						MTR_Parameter_write(PARAMETER_CONNECTION_ENPINPORT, MTR_Port_getUint32FromPin(&uartSettings->enPin));

						uint32_t connectionParameters = uartSettings->baud | uartSettings->parity << 16 | uartSettings->type << 24 | uartSettings->enableEnablePinInvesion << 28;
						MTR_Parameter_write(PARAMETER_CONNECTION_SPEED_AND_PARITY, connectionParameters);
						NVIC_SystemReset();
					}
				}
#endif


			break;

			case uartTimerRepeatesUserTimeout1:
				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;
				uartCommunication->callCallbacksMask |= (1 << uartTimerRepeatesUserTimeout1);
			break;

			case uartTimerRepeatesUserTimeout2:
				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;
				uartCommunication->callCallbacksMask |= (1 << uartTimerRepeatesUserTimeout2);
			break;

			case uartTimerRepeatesUserTimeout3:
				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;
				uartCommunication->callCallbacksMask |= (1 << uartTimerRepeatesUserTimeout3);
			break;

			case uartTimerRepeatesUserTimeout4:
				uartCommunication->needTimerNum = uartTimerRepeatesNotCounting;
				uartCommunication->callCallbacksMask |= (1 << uartTimerRepeatesUserTimeout4);

			break;
		}
	}
}

void MTR_Uart_clearOtherFlags(AvaliableConnectionType_t uart){

	Uart_SettingsStruct_t* uartSettings = &uartSettingsMainStructs[uart - Uart1Connection];

	UART_ClearITPendingBit(uartSettings->origin, UART_IT_BE);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_OE);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_PE);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_FE);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_RT);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_DSR);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_DCD);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_CTS);
	UART_ClearITPendingBit(uartSettings->origin, UART_IT_RI);

}

#ifdef CONFIG_UART_SERIAL_CAN_ENABLE
bool_t MTR_Uart_parseToCanMessage(MTR_Uart_CanRxMsg_t* messageTo, MTR_Uart_BufferStruct_t* buffer_, AvaliableConnectionType_t uart){

	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];

	uint32_t lenght = buffer_->amount;
	uint8_t* buffer = buffer_->buffer;

	if (lenght > 5 && buffer[lenght - 1] == crSymbol){
		switch (buffer[0]){
			case 'T':
			{
				if (lenght <= 27){

					//move ID to 0 tempID
					uint8_t tempId[4];
					charToHex(buffer + 1, 8, tempId);
					uint32_t idToSend = get32From8Bits(tempId);

					//move size to  place
					uint8_t tempLength[2];
					uint8_t lengthToSend;
					tempLength[0] = 0;
					tempLength[1] = buffer[9];
					charToHex(tempLength, 1, &lengthToSend);

					uint8_t tempData[8];
					uint32_t dataToSend[2];

					charToHex(buffer + 10, 8, tempData);

					dataToSend[0] = tempData[0] << 24 |  tempData[1] << 16 \
								  | tempData[2] << 8 | tempData[3];
					dataToSend[1] = tempData[4] << 24 |  tempData[5] << 16 \
								  | tempData[6] << 8 | tempData[7];

					if (lengthToSend < 9 && (lengthToSend * 2 + 11 == lenght))
					{
						messageTo->Rx_Header.ID = idToSend;
						messageTo->Rx_Header.length = lengthToSend;
						messageTo->Rx_Header.type = ExtendedUsual;
						messageTo->Data[0] = dataToSend[0];
						messageTo->Data[1] = dataToSend[1];
						return true;
					}
				}
			}
			break;
			case 't':
			{
				if (lenght <= 23 )
				{
					//move ID to 0 tempID
					uint8_t tempCharId[4];
					tempCharId[0] = 0;
					tempCharId[1] = buffer[1];
					tempCharId[2] = buffer[2];
					tempCharId[3] = buffer[3];

					uint8_t tempId[2];

					charToHex(tempCharId, 2, tempId);
					uint16_t idToSend = get16From8Bits(tempId);

					//move size to  place
					uint8_t tempLength[2];
					uint8_t lengthToSend;
					tempLength[0] = 0;
					tempLength[1] = buffer[4];
					charToHex(tempLength, 1, &lengthToSend);

					uint8_t tempData[8];
					uint32_t dataToSend[2];

					charToHex(buffer + 5, 8, tempData);

					dataToSend[0] = tempData[0] << 24 |  tempData[1] << 16 \
								  | tempData[2] << 8 | tempData[3];
					dataToSend[1] = tempData[4] << 24 |  tempData[5] << 16 \
								  | tempData[6] << 8 | tempData[7];

					if (lengthToSend < 9 && (lengthToSend * 2 + 6 == lenght))
					{
						messageTo->Rx_Header.ID = idToSend;
						messageTo->Rx_Header.length = lengthToSend;
						messageTo->Rx_Header.type = StandartUsual;
						messageTo->Data[0] = dataToSend[0];
						messageTo->Data[1] = dataToSend[1];
						return true;
					}
				}
			}
			break;

			case 'R':
			{
				if (lenght == 11)
				{
					//move ID to 0 tempID
					uint8_t tempId[4];
					charToHex(buffer + 1, 8, tempId);
					uint32_t idToSend = get32From8Bits(tempId);

					//move size to  place
					uint8_t tempLength[2];
					uint8_t lengthToSend;
					tempLength[0] = 0;
					tempLength[1] = buffer[9];
					charToHex(tempLength, 1, &lengthToSend);

					if (lengthToSend < 9)
					{
						messageTo->Rx_Header.ID = idToSend;
						messageTo->Rx_Header.length = lengthToSend;
						messageTo->Rx_Header.type= ExtendedRtr;
						return true;
					}
				}
			}
			break;
			case 'r':
			{
				if (lenght == 6 )
				{
					//move ID to 0 tempID
					uint8_t tempCharId[4];
					tempCharId[0] = 0;
					tempCharId[1] = buffer[1];
					tempCharId[2] = buffer[2];
					tempCharId[3] = buffer[3];

					uint8_t tempId[2];

					charToHex(tempCharId, 2, tempId);
					uint16_t idToSend = get16From8Bits(tempId);

					//move size to  place
					uint8_t tempLength[2];
					uint8_t lengthToSend;
					tempLength[0] = 0;
					tempLength[1] = buffer[4];
					charToHex(tempLength, 1, &lengthToSend);

					if (lengthToSend < 9)
					{
						messageTo->Rx_Header.ID = idToSend;
						messageTo->Rx_Header.length = lengthToSend;
						messageTo->Rx_Header.type = StandartRtr;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void MTR_Uart_addCanMessage(AvaliableConnectionType_t uart, MTR_Uart_CanRxMsg_t messageFrom)
{
	uint8_t dataSize;
	uint8_t tempTxBuffer[36];
	uint8_t tempHexFromCan[16];

	switch (messageFrom.Rx_Header.type) {
		case ExtendedRtr:
			//move from Can message to hex buffer

			set8From32Bits(tempHexFromCan, &messageFrom.Rx_Header.ID);
			hexToChar(&messageFrom.Rx_Header.length, 1, &tempTxBuffer[8]);
			//8 byte will be overwrited
			hexToChar(tempHexFromCan, 4, &tempTxBuffer[1]);
			tempTxBuffer[0] = 'R';
			tempTxBuffer[10] = crSymbol;

			dataSize = 11;
			break;

		case StandartRtr:
		{
			uint32_t badTempId = messageFrom.Rx_Header.ID;

			//11bit id is 2 bytes, but 3 symbol size!!!
			tempHexFromCan[0] = (uint8_t) (badTempId >> 8);
			tempHexFromCan[1] = (uint8_t) badTempId;

			hexToChar(&messageFrom.Rx_Header.length, 1, &tempTxBuffer[3]);

			//convert hex buffer to ASCII format
			hexToChar(tempHexFromCan, 2, tempTxBuffer);

			//overrid first unused pair byte for 'r'
			tempTxBuffer[0] = 'r';
			tempTxBuffer[5] = crSymbol;

			dataSize = 6;
		}
			break;

		case ExtendedUsual:
		{
			set8From32Bits(tempHexFromCan, &messageFrom.Rx_Header.ID);
			hexToChar(&messageFrom.Rx_Header.length, 1, &tempTxBuffer[8]);
			//8 byte will be overwrited
			hexToChar(tempHexFromCan, 4, &tempTxBuffer[1]);
			tempTxBuffer[0] = 'T';

			//move from Can message to hex buffer
			memcpy(&tempHexFromCan[5], (uint8_t*) messageFrom.Data, 8);
			hexToChar(&tempHexFromCan[5], (messageFrom.Rx_Header.length), &tempTxBuffer[10]);
			tempTxBuffer[10 + (messageFrom.Rx_Header.length * 2)] = crSymbol;

			dataSize = 11 + (messageFrom.Rx_Header.length * 2);
		}
			break;

		case StandartUsual:
		{
			uint32_t badTempId = messageFrom.Rx_Header.ID;

			//11bit id is 2 bytes, but 3 symbol size!!!
			tempHexFromCan[0] = (uint8_t) (badTempId >> 8);
			tempHexFromCan[1] = (uint8_t) badTempId;

			hexToChar(&messageFrom.Rx_Header.length, 1, &tempTxBuffer[3]);

			//convert hex buffer to ASCII format
			hexToChar(tempHexFromCan, 2, tempTxBuffer);
			memcpy(&tempHexFromCan[2], (uint8_t*) messageFrom.Data, 8);

			hexToChar(&tempHexFromCan[2], (messageFrom.Rx_Header.length), &tempTxBuffer[5]);

			//overrid first unused pair byte for 't'
			tempTxBuffer[0] = 't';
			tempTxBuffer[5 + (messageFrom.Rx_Header.length * 2)] = crSymbol;

			dataSize = 6 + (messageFrom.Rx_Header.length * 2);
		}
			break;
	}

	MTR_Uart_addArray(uart, tempTxBuffer, dataSize);
}
#endif

void MTR_Uart_rxInterruptHandler(AvaliableConnectionType_t uart, uint8_t byte){
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[uart - Uart1Connection];

	if (uartCommunication->direction == IN){
		MTR_Uart_addByteToRxBuffer(uart, byte);
		MTR_Uart_timerRefresh(uart, uartTimerRepeatesProccessingRecievedMessage);
	}
}

#ifdef CONFIG_UART1_ENABLE_LIBRARY_INTERRUPT

void UART1_IRQHandler(){

	//	Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(Uart1Connection);
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[0];

	if (MDR_UART1->RIS & UART_IT_RX){
		uint8_t byte = (uint8_t) MDR_UART1->DR;
		MTR_Uart_rxInterruptHandler(Uart1Connection, byte);
	}

	if (MDR_UART1->RIS & UART_IT_TX){
		if (uartCommunication->direction == OUT)
		{
			if (uartCommunication->txBuffer.b.amount != 0){
				MTR_Uart_sendNextByte(Uart1Connection);
			}
			else {
				MDR_UART1->ICR |= UART_IT_TX;
				MTR_Uart_timerRefresh(Uart1Connection, uartTimerRepeatesSwitchToReadAfterTransmit);
			}
		}
	}

	//reset error flags
	//MTR_Uart_clearOtherFlags(Uart1Connection);
	MDR_UART1->ICR |= UART_IT_BE | UART_IT_OE| UART_IT_PE| UART_IT_FE|UART_IT_RT|UART_IT_DSR|UART_IT_DCD|UART_IT_CTS |UART_IT_RI;

}
#endif

#ifdef CONFIG_UART2_ENABLE_LIBRARY_INTERRUPT

void UART2_IRQHandler(){

	//Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(Uart2Connection);
	Uart_CommunicationStruct_t* uartCommunication = uartCommunicationDataMainStructs[1];

	if (MDR_UART2->RIS & UART_IT_RX){
		uint8_t byte = (uint8_t) MDR_UART2->DR;
		MTR_Uart_rxInterruptHandler(Uart2Connection, byte);
	}

	if (MDR_UART2->RIS & UART_IT_TX){
		if (uartCommunication->direction == OUT)
		{
			if (uartCommunication->txBuffer.b.amount != 0){
				MTR_Uart_sendNextByte(Uart2Connection);
			}
			else {
				MDR_UART2->ICR |= UART_IT_TX;
				MTR_Uart_timerRefresh(Uart2Connection, uartTimerRepeatesSwitchToReadAfterTransmit);
			}
		}
	}

	//MTR_Uart_clearOtherFlags(Uart2Connection);
	//reset error flags
	MDR_UART2->ICR |= UART_IT_BE | UART_IT_OE| UART_IT_PE| UART_IT_FE|UART_IT_RT|UART_IT_DSR|UART_IT_DCD|UART_IT_CTS |UART_IT_RI;
}
#endif

#ifdef CONFIG_UART_TIMER_IRQ
void CONFIG_UART_TIMER_IRQ(){
	bool_t needCallCallbacks = false;
	for (uint8_t i = 0; i < UART_AMOUNT; i++){

		if( uartSettingsMainStructs[i].inited){
			if (uartCommunicationDataMainStructs[i]->needTimerNum != uartTimerRepeatesNotCounting)
			{
				if (uartCommunicationDataMainStructs[i]->timerCounterDown > 0){
					uartCommunicationDataMainStructs[i]->timerCounterDown -= 1;
				}
				else {
					MTR_Uart_updateTimerNumsFromIRQ(Uart1Connection + i);
				}
			}

			if (uartCommunicationDataMainStructs[i]->callCallbacksMask != 0)
			{
				needCallCallbacks = true;
			}
		}
	}

#ifndef CONFIG_UART_SELFCALL_CALLBACKS
	if (needCallCallbacks){
		CONFIG_UART_CALLBACKFUNC();
	}
#endif

	timer->CNT = 0;
	timer->STATUS &= ~TIMER_STATUS_CNT_ARR;

//	MTR_Timer_clearFlag(CONFIG_UART_TIMER, TIMER_STATUS_CNT_ARR);
//	MTR_Timer_setCounter(CONFIG_UART_TIMER, 0);
}

#endif

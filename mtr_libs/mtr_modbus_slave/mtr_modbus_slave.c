#include "mtr_modbus_slave/mtr_modbus_slave.h"
#include "mtr_crc/mtr_crc.h"
#include <malloc.h>
#include <string.h>


/**
 *  @brief Function of reading Holding Register (fc 0x03) from Holding Register Map.
 *  @param modbus source to operate.
 *  @return Nothing.
 */
static void MTR_Modbus_slave_processGetHoldingRegistersFunction(MTR_Modbus_t* modbus);


/**
 *  @brief Function of reading Input Register (fc 0x04) from Input Register Map.
 *  @param uart uart source to operate
 *  @return Nothing.
 */
static void MTR_Modbus_slave_processGetInputRegistersFunction(MTR_Modbus_t* modbus);


/**
 * @brief : Create response message. If request was good.
 * Function read rx_message from Modbus structure.
 * @param uart uart source to operate.
 * @return Nothing.
 *
 */
static void MTR_Modbus_slave_processRequest(MTR_Modbus_t* modbus);


/*
 * @brief : Verifier function for request.
 * @param uart struct to operate.
 * @return @ref MTR_ModbusRtu_error_t error.
 *
 */
static MTR_Modbus_error_t _MTR_Modbus_slave_requestVerify(MTR_Modbus_t* modbus);


/**
 * @brief : Send error message with need code
 * @param uart uart to operate
 * @param error Modbus error code
 * @return Nothing.
 */
static void MTR_Modbus_slave_sendErrorMessage(MTR_Modbus_t* modbus, MTR_Modbus_error_t  error);


MTR_Modbus_t* MTR_Modbus_slave_init(AvaliableConnectionType_t connection, uint8_t address)
{
	MTR_Modbus_t* modbusStruct = (MTR_Modbus_t*) calloc(sizeof(MTR_Modbus_t), 1);

	modbusStruct->connection = connection;
	modbusStruct->mode = MODBUS_MODE_SLAVE;
	modbusStruct->address = address;
	modbusStruct->specificModbusData = (MTR_Modbus_slave_specificData*) calloc(sizeof(MTR_Modbus_slave_specificData), 1);

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (connection == Uart1Connection || connection == Uart2Connection)
	{
		MTR_Uart_init(connection);

		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(connection);
		Uart_SettingsStruct_t* uartSettings = MTR_Uart_getSettingsStruct(connection);

		//set callbacks
		uartCommunication->switchToRecieve = MTR_ModbusRtu_uartSwitchToRecieve;
		uartCommunication->userReceive = MTR_Modbus_performHandler;

		MTR_Uart_initNvic(connection);
		MTR_Uart_enableInterrupts(connection);

		MTR_Uart_start(connection);

		MTR_Modbus_resetRxBuffer(modbusStruct);
	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (connection == Ethernet)
	{
		MTR_Tcp_port_t* port = MTR_Tcp_getPortStruct();

		MTR_Tcp_init(CONFIG_MODBUS_TCP_PORT);
		port->dataProcessingFunction = MTR_Modbus_Ethernet_performHandler;
		MTR_Ethernet_start(Ethernet);
	}
#endif

	MTR_Modbus_modbusElement* newModbusElement = (MTR_Modbus_modbusElement*) calloc(sizeof(MTR_Modbus_modbusElement), 1);

	if (!MTR_Modbus_startModbus()){
		MTR_Modbus_setStartModbus(newModbusElement);
	}
	else
	{
		MTR_Modbus_modbusElement* modbusElement = MTR_Modbus_startModbus();

		while (modbusElement->nextModbusElement) {
			modbusElement = modbusElement->nextModbusElement;
		}

		modbusElement->nextModbusElement = newModbusElement;
	}

	newModbusElement->modbus = modbusStruct;

	return modbusStruct;
}

errorType MTR_Modbus_slave_sendTxMessage(MTR_Modbus_t* modbus)
{
	 if (modbus->mode == MODBUS_MODE_SLAVE)
	 {
		if (modbus->txMessage.address == MODBUS_ADDRESS_BROADCAST) {
			return E_INVALID_VALUE;
		}

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
		if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
		{
			MTR_Uart_addByte(modbus->connection, modbus->txMessage.address);
			MTR_Uart_addByte(modbus->connection, modbus->txMessage.func);
			MTR_Uart_addArray(modbus->connection, (uint8_t*) &modbus->txMessage.data, modbus->txMessage.dataSize);
			MTR_Uart_addHalfWord(modbus->connection, swapBytes16(modbus->txMessage.crc));

			MTR_Uart_initTransmite(modbus->connection);

			modbus->totalTxPackages += 1;

			modbus->status = MODBUS_DISPATCH;
		}
#endif
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
		if (modbus->connection == Ethernet)
		{
			MTR_Tcp_sendData(
					(uint32_t)(MODBUS_TCP_HEADER_SIZE + modbus->txMessage.length),
					(uint8_t*)&modbus->txMessage.tidSwapped);
			modbus->totalTxPackages += 1;
			modbus->status = MODBUS_DISPATCH;
		}
#endif
	 }

	 return E_NO_ERROR;
}

void MTR_Modbus_slave_processGetHoldingRegistersFunction(MTR_Modbus_t* modbus)
{
	uint16_t quantity = 0;
	uint16_t startRegisterAddress = 0;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);

		uint8_t* buffer = uartCommunication->callbacksReceiveBuffer->buffer;

		quantity = buffer[5] + (buffer[4] << 8);
		startRegisterAddress =  buffer[3] + (buffer[2] << 8);

		modbus->txMessage.address = buffer[0];
		modbus->txMessage.func = buffer[1];
		modbus->txMessage.data[0] = 2 * quantity;
		modbus->txMessage.dataSize = 1;

		for (uint16_t i = 0; i < quantity; i += 1)
		{
			uint16_t registerData = *MTR_Modbus_slave_getLocalHoldingRegister(modbus, startRegisterAddress + i);

			modbus->txMessage.data[1 + 2*i] = (uint8_t) (registerData >> 8);
			modbus->txMessage.data[2 + 2*i] = (uint8_t) registerData;

		}
		modbus->txMessage.dataSize += (quantity * 2);

	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET

	if (modbus->connection == Ethernet)
	{
		startRegisterAddress = modbus->rxMessage.data[0] << 8 | modbus->rxMessage.data[1];
		quantity = modbus->rxMessage.data[2] << 8 | modbus->rxMessage.data[3];

		modbus->txMessage.tid = modbus->rxMessage.tid;
		modbus->txMessage.pid = modbus->rxMessage.pid;

		modbus->txMessage.address = modbus->rxMessage.address;
		modbus->txMessage.func = modbus->rxMessage.func;
		modbus->txMessage.data[0] = 2 * quantity;
		modbus->txMessage.dataSize = 1;

		for (uint16_t i = 0; i < quantity; i += 1) {

			uint16_t registerData = *MTR_Modbus_slave_getLocalHoldingRegister(modbus, startRegisterAddress + i);

			modbus->txMessage.data[1 + 2*i] = (uint8_t) (registerData >> 8);
			modbus->txMessage.data[2 + 2*i] = (uint8_t) registerData;

		}
		modbus->txMessage.dataSize += (quantity * 2);

		modbus->txMessage.length = modbus->txMessage.dataSize + 2;
	}
#endif
}

void MTR_Modbus_slave_processGetInputRegistersFunction(MTR_Modbus_t* modbus)
{
	uint16_t quantity;
	uint16_t startRegisterAddress;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL

	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		uint8_t* buffer = uartCommunication->callbacksReceiveBuffer->buffer;

		quantity = buffer[5] + (buffer[4] << 8);
		startRegisterAddress =  buffer[3] + (buffer[2] << 8);

		modbus->txMessage.address = buffer[0];
		modbus->txMessage.func = buffer[1];
		modbus->txMessage.data[0] = 2 * quantity;
		modbus->txMessage.dataSize = 1;

		for (uint16_t i = 0; i < quantity; i += 1){

			uint16_t registerData = *MTR_Modbus_slave_getLocalInputRegister(modbus, startRegisterAddress + i);

			modbus->txMessage.data[1 + 2*i] = (uint8_t) (registerData >> 8);
			modbus->txMessage.data[2 + 2*i] = (uint8_t) registerData;

		}

		modbus->txMessage.dataSize += (quantity * 2);

	}
#endif
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET

	if (modbus->connection == Ethernet)
	{
		startRegisterAddress = modbus->rxMessage.data[0] << 8 | modbus->rxMessage.data[1];
		quantity = modbus->rxMessage.data[2] << 8 | modbus->rxMessage.data[3];

		modbus->txMessage.tid = modbus->rxMessage.tid;
		modbus->txMessage.pid = modbus->rxMessage.pid;

		modbus->txMessage.address = modbus->rxMessage.address;
		modbus->txMessage.func = modbus->rxMessage.func;
		modbus->txMessage.data[0] = 2 * quantity;
		modbus->txMessage.dataSize = 1;

		for (uint16_t i = 0; i < quantity; i += 1){
			uint16_t registerData = *MTR_Modbus_slave_getLocalInputRegister(modbus, startRegisterAddress + i);
			modbus->txMessage.data[1 + 2*i] = (uint8_t) (registerData >> 8);
			modbus->txMessage.data[2 + 2*i] = (uint8_t) registerData;
		}

		modbus->txMessage.dataSize += (quantity * 2);

		modbus->txMessage.length = modbus->txMessage.dataSize + 2;
	}
#endif

}

void MTR_Modbus_slave_processSetSingleHoldingRegisterFunction(MTR_Modbus_t* modbus)
{
	uint16_t registerAddress;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL

	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		uint8_t* buffer = uartCommunication->callbacksReceiveBuffer->buffer;

		registerAddress =  buffer[3] + (buffer[2] << 8);

		uint16_t* varAddress = MTR_Modbus_slave_getLocalHoldingRegister(modbus, registerAddress);
		*varAddress = buffer[5] + (buffer[4] << 8);

		modbus->txMessage.address = buffer[0];
		modbus->txMessage.func = buffer[1];
		modbus->txMessage.data[0] =  buffer[2];
		modbus->txMessage.data[1] =  buffer[3];
		modbus->txMessage.data[2] =  (uint8_t) (*varAddress >> 8);
		modbus->txMessage.data[3] =  (uint8_t) *varAddress;
		modbus->txMessage.dataSize = 4;
	}
#endif
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET

	if (modbus->connection == Ethernet)
	{
		registerAddress =  (modbus->rxMessage.data[0] << 8) + modbus->rxMessage.data[1];

		uint16_t* varAddress = MTR_Modbus_slave_getLocalHoldingRegister(modbus, registerAddress);
		*varAddress =  (modbus->rxMessage.data[2] << 8) + modbus->rxMessage.data[3];

		modbus->txMessage.tid = modbus->rxMessage.tid;
		modbus->txMessage.pid = modbus->rxMessage.pid;

		modbus->txMessage.address = modbus->rxMessage.address;
		modbus->txMessage.func = modbus->rxMessage.func;
		modbus->txMessage.data[0] =  (uint8_t) registerAddress >> 8;
		modbus->txMessage.data[1] =  (uint8_t) registerAddress;
		modbus->txMessage.data[2] =  (uint8_t) (*varAddress >> 8);
		modbus->txMessage.data[3] =  (uint8_t) *varAddress;
		modbus->txMessage.dataSize = 4;

		modbus->txMessage.length = modbus->txMessage.dataSize + 2;
	}
#endif

}

void MTR_Modbus_slave_processSetLocalHoldingRegistersFunction(MTR_Modbus_t* modbus){

	uint16_t quantity;
	uint16_t startRegisterAddress;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		uint8_t* buffer = uartCommunication->callbacksReceiveBuffer->buffer;

		quantity = buffer[5] + (buffer[4] << 8);
		startRegisterAddress =  buffer[3] + (buffer[2] << 8);

		for (uint16_t i = 0; i < quantity; i++) {
			uint16_t* varAddress = MTR_Modbus_slave_getLocalHoldingRegister(modbus, startRegisterAddress + i);

			if (varAddress != 0) {
				*varAddress =  buffer[8 + 2*i] + (buffer[7 + 2*i] << 8);
			}
		}

		modbus->txMessage.address = buffer[0];
		modbus->txMessage.func = buffer[1];
		modbus->txMessage.data[0] =  buffer[2];
		modbus->txMessage.data[1] =  buffer[3];
		modbus->txMessage.data[2] =  buffer[4];
		modbus->txMessage.data[3] =  buffer[5];
		modbus->txMessage.dataSize = 4;
	}
#endif
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET

	if (modbus->connection == Ethernet)
	{
		startRegisterAddress = modbus->rxMessage.data[0] << 8 | modbus->rxMessage.data[1];
		quantity = (modbus->rxMessage.data[2] << 8) | (modbus->rxMessage.data[3]);

		for (uint16_t i = 0; i < quantity; i++){

			uint16_t* varAddress = MTR_Modbus_slave_getLocalHoldingRegister(modbus, startRegisterAddress + i);

			if (varAddress != 0){
				*varAddress =  (modbus->rxMessage.data[5 + 2*i] << 8) + (modbus->rxMessage.data[6 + 2*i]);
			}
		}

		modbus->txMessage.tid = modbus->rxMessage.tid;
		modbus->txMessage.pid = modbus->rxMessage.pid;

		modbus->txMessage.address = modbus->rxMessage.address;
		modbus->txMessage.func = modbus->rxMessage.func;
		modbus->txMessage.data[0] =  modbus->rxMessage.data[0];
		modbus->txMessage.data[1] =  modbus->rxMessage.data[1];
		modbus->txMessage.data[2] =  modbus->rxMessage.data[2];
		modbus->txMessage.data[3] =  modbus->rxMessage.data[3];
		modbus->txMessage.dataSize = 4;

		modbus->txMessage.length = modbus->txMessage.dataSize + 2;
	}
#endif
}


void MTR_Modbus_slave_processRequest(MTR_Modbus_t* modbus){

	uint8_t functionCode;

	MTR_Modbus_slave_callbackBeforeProccess(modbus);

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		functionCode = uartCommunication->callbacksReceiveBuffer->buffer[1];
	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (modbus->connection == Ethernet){
		functionCode = modbus->rxMessage.func;
	}
#endif

	switch (functionCode){

		case  MODBUS_FUNC_READ_HOLDING_REGS:
			MTR_Modbus_slave_processGetHoldingRegistersFunction(modbus);
			break;

		case MODBUS_FUNC_READ_INPUT_REGS:
			MTR_Modbus_slave_processGetInputRegistersFunction(modbus);
			break;

		case MODBUS_FUNC_WRITE_SINGLE_REG:
			MTR_Modbus_slave_processSetSingleHoldingRegisterFunction(modbus);
			break;

		case MODBUS_FUNC_WRITE_MULTIPLE_REGS:
			MTR_Modbus_slave_processSetLocalHoldingRegistersFunction(modbus);
			break;
	}


	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection){
		modbus->txMessage.crc = MTR_Crc_getCrc16Modbus((uint8_t*) &modbus->txMessage.address, MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + modbus->txMessage.dataSize);
	}

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (modbus->connection == Ethernet){
		modbus->txMessage.tidSwapped = swapBytes16(modbus->rxMessage.tid);
		modbus->txMessage.pidSwapped = swapBytes16(modbus->rxMessage.pid);
		modbus->txMessage.lengthSwapped = swapBytes16(modbus->txMessage.length);
	}
#endif

	MTR_Modbus_slave_callbackAfterProccess(modbus);

	MTR_Modbus_slave_sendTxMessage(modbus);

	modbus->status = MODBUS_DISPATCH;

}

void MTR_Modbus_slave_sendErrorMessage(MTR_Modbus_t* modbus, MTR_Modbus_error_t  error)
{
#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);

		modbus->txMessage.address = modbus->address;
		modbus->txMessage.func = 0x80 + uartCommunication->callbacksReceiveBuffer->buffer[1];
		modbus->txMessage.dataSize = 1;
		modbus->txMessage.data[0] = error;
		modbus->txMessage.crc = MTR_Crc_getCrc16Modbus((uint8_t*) &modbus->txMessage.address, MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_ERROR);

		MTR_Modbus_slave_sendTxMessage(modbus);
	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET

	if (modbus->connection == Ethernet)
	{
		MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();

		modbus->rxMessage.tid = (uint16_t)(tcpData->data[0] << 8 | tcpData->data[1]);
		modbus->rxMessage.pid = (uint16_t)(tcpData->data[2] << 8 | tcpData->data[3]);

		modbus->txMessage.tid = modbus->rxMessage.tid;
		modbus->txMessage.pid = modbus->rxMessage.pid;

		modbus->txMessage.tidSwapped = swapBytes16(modbus->rxMessage.tid);
		modbus->txMessage.pidSwapped = swapBytes16(modbus->rxMessage.pid);

		modbus->txMessage.address = modbus->address;
		modbus->txMessage.func = 0x80 + tcpData->data[1 + MODBUS_TCP_HEADER_SIZE];
		modbus->txMessage.dataSize = 1;
		modbus->txMessage.data[0] = error;

		modbus->txMessage.length = modbus->txMessage.dataSize + 2;
		modbus->txMessage.lengthSwapped = swapBytes16(modbus->txMessage.length);

		MTR_Modbus_slave_sendTxMessage(modbus);
	}
#endif

}


errorType MTR_Modbus_slave_addHoldingRegister(MTR_Modbus_t* modbus, uint8_t address, uint16_t* variableAddress){

	MTR_Modbus_slave_specificData* specificData = modbus->specificModbusData;

	if (specificData->currentHoldingRegisterIndex >= CONFIG_MODBUS_MAX_AMOUNT_HOLDING_REGISTERS_PER_SLAVE){
		return E_OUT_OF_RANGE;
	}

	//maybe register is already added
	for (uint16_t i = 0; i < CONFIG_MODBUS_MAX_AMOUNT_HOLDING_REGISTERS_PER_SLAVE; i++){

		if (specificData->holdingRegisterMap[i].address == address && specificData->holdingRegisterMap[i].inited)
		{
			specificData->holdingRegisterMap[i].variableAddress = variableAddress;
			return E_NO_ERROR;
		}
	}

	specificData->holdingRegisterMap[specificData->currentHoldingRegisterIndex].address = address;
	specificData->holdingRegisterMap[specificData->currentHoldingRegisterIndex].variableAddress = variableAddress;
	specificData->holdingRegisterMap[specificData->currentHoldingRegisterIndex].inited = true;

	specificData->currentHoldingRegisterIndex++;
	return E_NO_ERROR;
}

errorType MTR_Modbus_slave_addInputRegister(MTR_Modbus_t* modbus, uint8_t address, uint16_t* variableAddress){

	MTR_Modbus_slave_specificData* specificData = modbus->specificModbusData;

	if (specificData->currentInputRegisterIndex >= CONFIG_MODBUS_MAX_AMOUNT_INPUT_REGISTERS_PER_SLAVE){
		return E_OUT_OF_RANGE;
	}

	//maybe register already exist
	for (uint16_t i = 0; i < CONFIG_MODBUS_MAX_AMOUNT_INPUT_REGISTERS_PER_SLAVE; i++){

		if (specificData->inputRegisterMap[i].address == address && specificData->inputRegisterMap[i].inited)
		{
			specificData->inputRegisterMap[i].variableAddress = variableAddress;
			return E_NO_ERROR;
		}
	}

	specificData->inputRegisterMap[specificData->currentInputRegisterIndex].address = address;
	specificData->inputRegisterMap[specificData->currentInputRegisterIndex].variableAddress = variableAddress;
	specificData->inputRegisterMap[specificData->currentInputRegisterIndex].inited = true;
	specificData->currentInputRegisterIndex++;
	return E_NO_ERROR;

}

uint16_t* MTR_Modbus_slave_getLocalHoldingRegister(MTR_Modbus_t* modbus, uint16_t address){

	MTR_Modbus_slave_specificData* specificData = modbus->specificModbusData;

	static uint16_t i = 0;
	uint16_t j = 0;

	for (; j < CONFIG_MODBUS_MAX_AMOUNT_HOLDING_REGISTERS_PER_SLAVE; j++){

		if (i >= CONFIG_MODBUS_MAX_AMOUNT_HOLDING_REGISTERS_PER_SLAVE){
			i = 0;
		}

		if ((specificData->holdingRegisterMap[i].address == address) && specificData->holdingRegisterMap[i].inited){
			return specificData->holdingRegisterMap[i].variableAddress;
		}

		i++;
	}

	return 0;
}


uint16_t* MTR_Modbus_slave_getLocalInputRegister(MTR_Modbus_t* modbus, uint16_t address){

	MTR_Modbus_slave_specificData* specificData = modbus->specificModbusData;

	uint16_t j = 0;
	static uint16_t i = 0;

	for (; j < CONFIG_MODBUS_MAX_AMOUNT_INPUT_REGISTERS_PER_SLAVE; j++){

		if (i >= CONFIG_MODBUS_MAX_AMOUNT_INPUT_REGISTERS_PER_SLAVE){
			i = 0;
		}

		if ((specificData->inputRegisterMap[i].address == address) && specificData->inputRegisterMap[i].inited){
			return specificData->inputRegisterMap[i].variableAddress;
		}

		i++;
	}

	return 0;
}


MTR_Modbus_error_t _MTR_Modbus_slave_requestVerify(MTR_Modbus_t* modbus)
{
	if (modbus->mode == MODBUS_MODE_SLAVE)
	{

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
		if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
		{
			Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
			uint8_t* buffer = uartCommunication->callbacksReceiveBuffer->buffer;

			uint16_t startAddress = buffer[2] << 8 | buffer[3];
			uint16_t quantity = ((buffer[4] * 256) + buffer[5]);

			switch (buffer[1]){

				case MODBUS_FUNC_READ_HOLDING_REGS:
					break;

				case MODBUS_FUNC_READ_INPUT_REGS:
					break;

				case MODBUS_FUNC_WRITE_SINGLE_REG:
					quantity = 1;
					break;

				case MODBUS_FUNC_WRITE_MULTIPLE_REGS:
					break;

				default :
					return MODBUS_ERROR_INVALID_FUNC;
			}

			if (!(quantity >= 1 && quantity <= 0x7D)) {
				return MODBUS_ERROR_INVALID_ADDRESS;
			}

			//Get first register address and quantity from rx buffer

			MTR_Modbus_slave_specificData* specificData = modbus->specificModbusData;

			if (buffer[1] == MODBUS_FUNC_READ_HOLDING_REGS || buffer[1] \
					== MODBUS_FUNC_WRITE_SINGLE_REG || buffer[1] == MODBUS_FUNC_WRITE_MULTIPLE_REGS)
			{
				for (uint16_t i = 0; i < quantity; i++) {
					if (MTR_Modbus_slave_getLocalHoldingRegister(modbus, startAddress + i) == 0){
						return MODBUS_ERROR_INVALID_ADDRESS;
					}
				}
			}
			else
			{
				if (buffer[1] == MODBUS_FUNC_READ_INPUT_REGS)
				{
					for (uint16_t i = 0; i < quantity; i++){
						if (MTR_Modbus_slave_getLocalInputRegister(modbus, startAddress + i) == 0){
							return MODBUS_ERROR_INVALID_ADDRESS;
						}
					}
				}
			}

			modbus->status = MODBUS_CONFIRM;
			return MODBUS_ERROR_NONE;
		}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
		//ETHERNET
		if (modbus->connection == Ethernet){

			MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();
			uint8_t* tcpBuffer = &tcpData->data[MODBUS_TCP_HEADER_SIZE];

			uint16_t startAddress = tcpBuffer[2] << 8 | tcpBuffer[3];
			uint16_t quantity = tcpBuffer[4] << 8 | tcpBuffer[5];

			switch (tcpBuffer[1]){

				case MODBUS_FUNC_READ_HOLDING_REGS:
					break;

				case MODBUS_FUNC_READ_INPUT_REGS:
					break;

				case MODBUS_FUNC_WRITE_SINGLE_REG:
					quantity = 1;
					break;

				case MODBUS_FUNC_WRITE_MULTIPLE_REGS:
					break;

				default :
					return MODBUS_ERROR_INVALID_FUNC;
			}

			if (!(quantity >= 1 && quantity <= 0x7D)) {
				return MODBUS_ERROR_INVALID_ADDRESS;
			}

			MTR_Modbus_slave_specificData* specificData = modbus->specificModbusData;

			if (tcpBuffer[1] == MODBUS_FUNC_READ_HOLDING_REGS || tcpBuffer[1] == MODBUS_FUNC_WRITE_SINGLE_REG \
					|| tcpBuffer[1] == MODBUS_FUNC_WRITE_MULTIPLE_REGS)
			{
				for (uint16_t i = 0; i < quantity; i++) {
					if (MTR_Modbus_slave_getLocalHoldingRegister(modbus, startAddress + i) == 0){
						return MODBUS_ERROR_INVALID_ADDRESS;
					}
				}
			}
			else
			{
				if (tcpBuffer[1] == MODBUS_FUNC_READ_INPUT_REGS)
				{
					for (uint16_t i = 0; i < quantity; i++) {
						if (MTR_Modbus_slave_getLocalInputRegister(modbus, startAddress + i) == 0){
							return MODBUS_ERROR_INVALID_ADDRESS;
						}
					}
				}
			}

			modbus->status = MODBUS_CONFIRM;
			return MODBUS_ERROR_NONE;
		}
#endif
	}

	return 0xFF;
}


bool_t MTR_Modbus_slave_sizeTestMessage(MTR_Modbus_t* modbus){

#ifndef CONFIG_MODBUS_DISABLE_SERIAL

	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		MTR_Uart_BufferStruct_t* buffer = uartCommunication->callbacksReceiveBuffer;

		uint8_t fc = buffer->buffer[1];
		uint16_t quantity = (( buffer->buffer[4] * 256) + buffer->buffer[5]);

		switch (fc){

			case MODBUS_FUNC_READ_COILS_STATUS:
				return (buffer->amount == 8);

			case MODBUS_FUNC_READ_DISCR_INPUTS:
				return (buffer->amount == 8);

			case MODBUS_FUNC_WRITE_SINGLE_COIL:
				return (buffer->amount == 8);

			case MODBUS_FUNC_WRITE_MULTIPLE_COILS:
				return ((bitesToBytes(quantity) == buffer->buffer[6]) && \
							(buffer->amount == 9 + buffer->buffer[6]));

			case MODBUS_FUNC_READ_HOLDING_REGS:
				return (buffer->amount == 8);

			case MODBUS_FUNC_READ_INPUT_REGS:
				return (buffer->amount == 8);

			case MODBUS_FUNC_WRITE_SINGLE_REG:
				return (buffer->amount == 8);

			case MODBUS_FUNC_WRITE_MULTIPLE_REGS:
				return (buffer->amount == MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + \
						MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY + MODBUS_BYTE_SIZE_NEXT_BYTES + \
						MODBUS_BYTE_SIZE_CRC + quantity * 2);
			default :
				return false;
		}
	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (modbus->connection == Ethernet)
	{
		MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();
		uint8_t fc = tcpData->data[1 + MODBUS_TCP_HEADER_SIZE];
		uint16_t quantity = ((tcpData->data[4 + MODBUS_TCP_HEADER_SIZE] * 256) + \
				tcpData->data[5 + MODBUS_TCP_HEADER_SIZE]);

		switch (fc){

			case MODBUS_FUNC_READ_COILS_STATUS:
				return (tcpData->size == 6 + MODBUS_TCP_HEADER_SIZE);

			case MODBUS_FUNC_READ_DISCR_INPUTS:
				return (tcpData->size == 6 + MODBUS_TCP_HEADER_SIZE);

			case MODBUS_FUNC_WRITE_SINGLE_COIL:
				return (tcpData->size == 6 + MODBUS_TCP_HEADER_SIZE);

			case MODBUS_FUNC_WRITE_MULTIPLE_COILS:	//** fix the nine
				return ((bitesToBytes(quantity) == tcpData->data[6 + MODBUS_TCP_HEADER_SIZE]) && \
							(tcpData->size == 9 + tcpData->data[6 + MODBUS_TCP_HEADER_SIZE]));

			case MODBUS_FUNC_READ_HOLDING_REGS:
				return (tcpData->size == 6 + MODBUS_TCP_HEADER_SIZE);

			case MODBUS_FUNC_READ_INPUT_REGS:
				return (tcpData->size == 6 + MODBUS_TCP_HEADER_SIZE);

			case MODBUS_FUNC_WRITE_SINGLE_REG:
				return (tcpData->size == 6 + MODBUS_TCP_HEADER_SIZE);

			case MODBUS_FUNC_WRITE_MULTIPLE_REGS:
				return (tcpData->size == MODBUS_TCP_HEADER_SIZE + MODBUS_BYTE_SIZE_ADDRESS + \
						MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + \
						MODBUS_BYTE_SIZE_QUANTITY + MODBUS_BYTE_SIZE_NEXT_BYTES + quantity * 2);
			default :
				return false;
		}
	}
#endif

	return false;
}

bool_t MTR_Modbus_slave_requestHandler(MTR_Modbus_t* modbus){

	bool_t returnValue = false;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		MTR_Uart_BufferStruct_t* buffer = uartCommunication->callbacksReceiveBuffer;

		if ((buffer->buffer[0] == modbus->address) && buffer->amount >= MODBUS_RTU_PACKET_SIZE_MIN)
		{
			modbus->totalRxPackages +=1;
			modbus->rxMessage.checked = false;

			//CRC test
			uint16_t crc = MTR_Crc_getCrc16Modbus((uint8_t*) buffer->buffer, buffer->amount - 2);
			uint16_t crc_temp = buffer->buffer[buffer->amount - 2] |\
										(buffer->buffer[buffer->amount - 1] << 8);

			if (crc_temp == crc && MTR_Modbus_slave_sizeTestMessage(modbus))
			{
				//crc passed, check data now
				MTR_Modbus_error_t error = _MTR_Modbus_slave_requestVerify(modbus);

				if (error == MODBUS_ERROR_NONE)
				{
					modbus->totalRxGoodPackages +=1;

					modbus->rxMessage.address = buffer->buffer[0];
					modbus->rxMessage.func = buffer->buffer[1];

					memcpy(modbus->rxMessage.data, buffer->buffer + 2, buffer->amount - 4);
					modbus->rxMessage.dataSize = buffer->amount - 4;
					modbus->rxMessage.crc = crc;
					modbus->rxMessage.checked = true;
					returnValue = true;

					//Process request
					MTR_Modbus_slave_processRequest(modbus);
				}
				else
				{
					if (error != 0xFF){
						modbus->totalRxErrorPackages +=1;
						MTR_Modbus_slave_sendErrorMessage(modbus, error);
					}
				}
			}
			// fix: delete last zero byte
			else if(buffer->buffer[buffer->amount - 1] == 0){
				buffer->amount--;
				returnValue = MTR_Modbus_slave_requestHandler(modbus);
			}
			else
			{
				modbus->totalRxErrorPackages +=1;
			}
		}

		MTR_Modbus_resetRxBuffer(modbus);
	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
 	if (modbus->connection == Ethernet)
	{

		MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();

		if ((tcpData->data[MODBUS_TCP_HEADER_SIZE] == modbus->address) && tcpData->size >= MODBUS_TCP_PACKET_SIZE_MIN)
		{
			modbus->totalRxPackages +=1;
			modbus->rxMessage.checked = false;

			if (MTR_Modbus_slave_sizeTestMessage(modbus))
			{
				MTR_Modbus_error_t error = _MTR_Modbus_slave_requestVerify(modbus);

				if (error == MODBUS_ERROR_NONE)
				{
					modbus->totalRxGoodPackages +=1;

					modbus->rxMessage.tid = (uint16_t)(tcpData->data[0] << 8 | tcpData->data[1]);
					modbus->rxMessage.pid = (uint16_t)(tcpData->data[2] << 8 | tcpData->data[3]);
					modbus->rxMessage.length = (uint16_t)(tcpData->data[4] << 8 | tcpData->data[5]);

					modbus->rxMessage.address = tcpData->data[0 + MODBUS_TCP_HEADER_SIZE];
					modbus->rxMessage.func = tcpData->data[1 + MODBUS_TCP_HEADER_SIZE];

					memcpy(modbus->rxMessage.data, tcpData->data + \
							2 + MODBUS_TCP_HEADER_SIZE, tcpData->size - 2 - MODBUS_TCP_HEADER_SIZE);
					modbus->rxMessage.dataSize = tcpData->size - 2 - MODBUS_TCP_HEADER_SIZE;
					modbus->rxMessage.checked = true;
					returnValue = true;

					//Process request
					MTR_Modbus_slave_processRequest(modbus);
				}
				else
				{
					if (error != 0xFF){
						modbus->totalRxErrorPackages +=1;
						MTR_Modbus_slave_sendErrorMessage(modbus, error);
					}
				}



			}
			// fix: delete last zero byte
			else if(tcpData->data[tcpData->size - 1] == 0){
				tcpData->size--;
				returnValue = MTR_Modbus_slave_requestHandler(modbus);
			}
			else
			{
				modbus->totalRxErrorPackages +=1;
			}
		}

	}
#endif


	return returnValue;
}

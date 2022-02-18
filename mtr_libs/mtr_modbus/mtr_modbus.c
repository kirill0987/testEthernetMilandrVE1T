#include "mtr_modbus/mtr_modbus.h"
#include "mtr_crc/mtr_crc.h"

MTR_Modbus_t* MTR_Modbus_master_init(AvaliableConnectionType_t connection)
{
	MTR_Modbus_t* modbusStruct = calloc(sizeof(MTR_Modbus_t), 1);
	modbusStruct->specificModbusData = calloc(sizeof(MTR_Modbus_master_specificData), 1);
	modbusStruct->connection = connection;
	modbusStruct->mode = MODBUS_MODE_MASTER;

	MTR_Modbus_master_specificData* specificData = modbusStruct->specificModbusData;
#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (connection == Uart1Connection || connection == Uart2Connection)
	{
		MTR_Uart_init(connection);

		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(connection);
		Uart_SettingsStruct_t* uartSettings = MTR_Uart_getSettingsStruct(connection);

		MTR_Modbus_resetRxBuffer(modbusStruct);

		MTR_Uart_setNumOfRepeats(connection, uartTimerRepeatesUserTimeout1, ((MTR_Uart_getTimerBaudSpeed()/uartSettings->baud )*(MTR_Uart_getTimerBaudSpeed() / (8 * 1000)) * CONFIG_MODBUS_RTU_TIMEOUT_MS));	//CONFIG_MODBUS_RTU_TIMEOUT_MS device timeout

		uartCommunication->switchToRecieve = MTR_ModbusRtu_uartSwitchToRecieve;
		uartCommunication->userReceive = MTR_Modbus_performHandler;
		uartCommunication->userTimeout1 = MTR_Modbus_performHandler;

		specificData->pollingHoldingRegisters = calloc(sizeof(MTR_Modbus_pollingHoldingRegisters_t), 1);
		specificData->pollingInputRegisters = calloc(sizeof(MTR_Modbus_pollingInputRegisters_t), 1);

		MTR_Uart_initNvic(connection);
		MTR_Uart_enableInterrupts(connection);
		MTR_Uart_start(connection);
	}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (connection == Ethernet)
	{
		MTR_Tcp_port_t* port = MTR_Tcp_getPortStruct();

		modbusStruct->txMessage.tid = 0;
		MTR_Tcp_init (TCP_PORT_MODBUS);
		port->dataProcessingFunction = MTR_Modbus_performHandler;
		MTR_Ethernet_start(Ethernet);
	}
#endif

	MTR_Modbus_modbusElement* newModbusElement = calloc(sizeof(MTR_Modbus_modbusElement), 1);

	if (!MTR_Modbus_startModbus()){
		MTR_Modbus_setStartModbus(newModbusElement);
	}
	else
	{
		MTR_Modbus_modbusElement* modbusElement = MTR_Modbus_startModbus();

		while(modbusElement->nextModbusElement){
			modbusElement = modbusElement->nextModbusElement;
		}

		modbusElement->nextModbusElement = newModbusElement;
	}

	newModbusElement->modbus = modbusStruct;

	return modbusStruct;
}

MTR_Modbus_device_t* MTR_Modbus_master_getDevice(MTR_Modbus_t* modbus, uint8_t deviceAddress)
{
	uint16_t i;
	bool_t deviceNotFound = true;

	MTR_Modbus_master_specificData* specificData = modbus->specificModbusData;

	for (i = 0; i < specificData->connections.currentDeviceNum; i++){

		if (specificData->connections.device[i].address == deviceAddress){
			deviceNotFound = false;
			break;
		}
	}

	if (deviceNotFound) {
		//add new device and return
		if (specificData->connections.currentDeviceNum < CONFIG_MODBUS_CONNECTED_DEVICE_NUM)
		{
			toZeroByteFunction(specificData->connections.device + specificData->connections.currentDeviceNum , sizeof(MTR_Modbus_device_t));
			specificData->connections.device[specificData->connections.currentDeviceNum].address = deviceAddress;
			i = specificData->connections.currentDeviceNum;
			specificData->connections.currentDeviceNum += 1;
			deviceNotFound = false;
		}
	}

	if (!deviceNotFound) {
		return &specificData->connections.device[i];
	}
	else {
		return 0;
	}
}

errorType MTR_Modbus_master_sendTxMessage(MTR_Modbus_t* modbus)
{
	if (modbus->mode == MODBUS_MODE_MASTER)
	{
		if (modbus->status == MODBUS_PREPARATION)
		{
			if (modbus->txMessage.address== MODBUS_ADDRESS_BROADCAST){
				return E_INVALID_VALUE;
			}

			MTR_Modbus_master_specificData* specificData = modbus->specificModbusData;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
			if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
			{
				MTR_Uart_addArray(modbus->connection, (uint8_t*) &modbus->txMessage.address, MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY + modbus->txMessage.dataSize);
				MTR_Uart_addArray(modbus->connection, (uint8_t*) &modbus->txMessage.crc, MODBUS_RTU_BYTE_SIZE_CRC);
				MTR_Uart_initTransmite(modbus->connection);

				MTR_Modbus_device_t* device = MTR_Modbus_master_getDevice(modbus, modbus->txMessage.address);

				if (device){
					device->txCounter += 1;
				}

				modbus->totalTxPackages += 1;
				modbus->status = MODBUS_DISPATCH;

				return E_NO_ERROR;
			}
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
			if (modbus->connection == Ethernet)
			{
				MTR_Tcp_sendData ((uint32_t)(MODBUS_TCP_HEADER_SIZE + modbus->txMessage.length),\
										(uint8_t*) &modbus->txMessage.tid);

				MTR_Modbus_device_t* device = MTR_Modbus_master_getDevice(modbus, modbus->txMessage.address);

				if (device){
					device->txCounter += 1;
				}

				modbus->totalTxPackages += 1;
				modbus->status = MODBUS_DISPATCH;

				return E_NO_ERROR;
			}
#endif
		}
		else {
			return E_STATE;
		}

	}

    return E_NOT_IMPLEMENTED;
}

uint16_t* MTR_Modbus_master_addPollingRegister(MTR_Modbus_t* modbus, uint8_t slaveAddress, MTR_Modbus_registerType_t registerType, uint16_t registerAddress)
{
	uint8_t i = 0;
	MTR_Modbus_pollingRegister_t* pollingRegisters;
	uint16_t currentPollingRegistersNumber;
	uint16_t maximumPollingRegistersNumber;

	MTR_Modbus_master_specificData* specificData = modbus->specificModbusData;

	switch (registerType)
	{
		case INPUT:
			pollingRegisters = specificData->pollingInputRegisters->registers;
			currentPollingRegistersNumber = specificData->pollingInputRegisters->currentPollingRegistersNumber;
			maximumPollingRegistersNumber = CONFIG_MODBUS_MAX_PULLING_INPUT_REGISTER_NUM_PER_CONNECTION;
			break;
		case HOLDING:
			pollingRegisters = specificData->pollingHoldingRegisters->registers;
			currentPollingRegistersNumber = specificData->pollingHoldingRegisters->currentPollingRegistersNumber;
			maximumPollingRegistersNumber = CONFIG_MODBUS_MAX_PULLING_HOLDING_REGISTER_NUM_PER_CONNECTION;
			break;
	}

	for (i = 0; i < currentPollingRegistersNumber; i++) {
		if (pollingRegisters[i].parentModbus != modbus || pollingRegisters[i].slaveAddress != slaveAddress || pollingRegisters[i].registerAddress != registerAddress){
			continue;
		}
		break;
	}

	//register is not polling yet
	if (i == currentPollingRegistersNumber){
		if (currentPollingRegistersNumber < maximumPollingRegistersNumber){
			//add new polling register
			pollingRegisters[currentPollingRegistersNumber].parentModbus = modbus;
			pollingRegisters[currentPollingRegistersNumber].slaveAddress = slaveAddress;
			pollingRegisters[currentPollingRegistersNumber].registerAddress = registerAddress;
			pollingRegisters[currentPollingRegistersNumber].registerValue = 0;

			if(registerType == INPUT){
				specificData->pollingInputRegisters->currentPollingRegistersNumber += 1;
			}
			else {
				specificData->pollingHoldingRegisters->currentPollingRegistersNumber += 1;
			}

			return &pollingRegisters[currentPollingRegistersNumber].registerValue;
		}
		//out of memory
		return 0;
	}

	return &pollingRegisters[currentPollingRegistersNumber - 1].registerValue;
}

uint16_t* MTR_Modbus_master_getPollingRegister(MTR_Modbus_t* modbus, uint8_t slaveAddress, MTR_Modbus_registerType_t registerType, uint16_t registerAddress)
{
	uint8_t i = 0;
	MTR_Modbus_pollingRegister_t * pollingRegisters;
	uint8_t currentPollingRegistersNumber;

	MTR_Modbus_master_specificData* specificData = modbus->specificModbusData;

	switch (registerType) {
		case INPUT:
			pollingRegisters = specificData->pollingInputRegisters->registers;
			currentPollingRegistersNumber = specificData->pollingInputRegisters->currentPollingRegistersNumber;
			break;
		case HOLDING:
			pollingRegisters = specificData->pollingHoldingRegisters->registers;
			currentPollingRegistersNumber = specificData->pollingHoldingRegisters->currentPollingRegistersNumber;
			break;
		default:
			return 0;
	}

	for (i = 0; i < currentPollingRegistersNumber; i++){

		if (pollingRegisters[i].registerAddress != registerAddress) {
			continue;
		}
		if (pollingRegisters[i].slaveAddress != slaveAddress ){
			continue;
		}

		if (pollingRegisters[i].parentModbus != modbus){
			continue;
		}

		return &pollingRegisters[i].registerValue;
	}

	//add register to map
	return MTR_Modbus_master_addPollingRegister(modbus, slaveAddress, registerType, registerAddress);
}

errorType MTR_Modbus_master_initMessage(MTR_Modbus_t* modbus, uint8_t function, uint8_t address, uint16_t startRegister, uint8_t quantity, uint16_t* data)
{
#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
	{
		if (modbus->status == MODBUS_READY)
		{
			//Change status to preparation
			modbus->status = MODBUS_PREPARATION;

			//Reset data size
			modbus->txMessage.dataSize = 0;

			//Build txMessage using given parameters
			modbus->txMessage.address = address;
			modbus->txMessage.func = function;

			modbus->txMessage.data[0] = (startRegister >> 8) & 0xFF;
			modbus->txMessage.data[1] = startRegister & 0xFF;

			_MTR_Modbus_master_setMessageData(modbus, function, quantity, data);
			modbus->txMessage.crc = MTR_Crc_getCrc16Modbus((uint8_t*) &modbus->txMessage.address, MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_QUANTITY + MODBUS_BYTE_SIZE_START_REGISTER + modbus->txMessage.dataSize);

			MTR_Modbus_master_sendTxMessage(modbus);

			return E_NO_ERROR;
		}
		return E_BUSY;
    }
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (modbus->connection == Ethernet)
	{
		if (modbus->status == MODBUS_READY)
		{
			//Change status to preparation
			modbus->status = MODBUS_PREPARATION;

			//Reset data size
			modbus->txMessage.dataSize = 0;

			//Build txMessage using given parameters
			modbus->txMessage.tid++;
			modbus->txMessage.pid = (uint16_t) MODBUS_TCP_PROTOCOL_ID;

			modbus->txMessage.address = address;
			modbus->txMessage.func = function;

			modbus->txMessage.data[0] = (startRegister >> 8) & 0xFF;
			modbus->txMessage.data[1] = startRegister & 0xFF;

			_MTR_Modbus_master_setMessageData(modbus, function, quantity, data);
			modbus->txMessage.length = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY + modbus->txMessage.dataSize;

			MTR_Modbus_master_sendTxMessage(modbus);

			return E_NO_ERROR;
		}
		return E_BUSY;
    }
#endif

	return E_NOT_IMPLEMENTED;
}

errorType MTR_Modbus_master_getHoldingRegisters(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint8_t quantity)
{
    return MTR_Modbus_master_initMessage(modbus, MODBUS_FUNC_READ_HOLDING_REGS, address, startRegister, quantity, 0);
}

errorType MTR_Modbus_master_getInputRegisters(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint8_t quantity)
{
	return MTR_Modbus_master_initMessage(modbus, MODBUS_FUNC_READ_INPUT_REGS, address, startRegister, quantity, 0);
}

errorType MTR_Modbus_master_setRegister(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint16_t* data)
{
	return MTR_Modbus_master_initMessage(modbus, MODBUS_FUNC_WRITE_SINGLE_REG, address, startRegister, 1, data);
}

errorType MTR_Modbus_master_setRegisters(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint8_t quantity, uint16_t* data)
{
	return MTR_Modbus_master_initMessage(modbus, MODBUS_FUNC_WRITE_MULTIPLE_REGS, address, startRegister, quantity, data);
}

void _MTR_Modbus_master_setMessageData(MTR_Modbus_t* modbus, uint8_t function, uint8_t quantity, uint16_t* data)
{
    if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection || modbus->connection == Ethernet)
    {
        uint16_t dataSizeBytes = 0;

        switch (function){

        case (MODBUS_FUNC_READ_COILS_STATUS):
            modbus->txMessage.data [2] = 0;
            modbus->txMessage.data [3] = quantity;
            break;

        case (MODBUS_FUNC_READ_DISCR_INPUTS):
            modbus->txMessage.data [2] = 0;
            modbus->txMessage.data [3] = quantity;
            break;

        case (MODBUS_FUNC_READ_HOLDING_REGS):
            modbus->txMessage.data [2] = 0;
            modbus->txMessage.data [3] = quantity;
            break;


        case (MODBUS_FUNC_READ_INPUT_REGS):
            modbus->txMessage.data [2] = 0;
            modbus->txMessage.data [3] = quantity;
            break;

        case (MODBUS_FUNC_WRITE_SINGLE_COIL):
            modbus->txMessage.data [2] = data [0];
            modbus->txMessage.data [3] = 0;
            break;

        case (MODBUS_FUNC_WRITE_SINGLE_REG):
            modbus->txMessage.data [2] = (uint8_t) (data[0] >> 8);
            modbus->txMessage.data [3] = (uint8_t) data[0];
            break;

        case (MODBUS_FUNC_WRITE_MULTIPLE_COILS):
            modbus->txMessage.data[2] = 0;
            modbus->txMessage.data[3] = quantity;

            dataSizeBytes = bitesToBytes(quantity);

            modbus->txMessage.data[4] = dataSizeBytes;
            modbus->txMessage.dataSize +=1;

            for (int i = 0; i < dataSizeBytes; i+=2)
            {
                if ((dataSizeBytes - i) == 1)
                {
                    modbus->txMessage.data[5+i] = (uint8_t) (data[i/2] >> 8);
                    modbus->txMessage.dataSize +=1;
                }
                else
                {
                    modbus->txMessage.data[5+i] = (uint8_t) data[i/2];
                    modbus->txMessage.data[6+i] = (uint8_t) (data[i/2] >> 8);
                    modbus->txMessage.dataSize +=2;
                }
            }
            break;

        case (MODBUS_FUNC_WRITE_MULTIPLE_REGS):
            modbus->txMessage.data[2] = 0;
            modbus->txMessage.data[3] = quantity;

            modbus->txMessage.data[4] = quantity * 2;
            modbus->txMessage.dataSize +=1;

            for (int i = 0; i < modbus->txMessage.data[4]; i+=2) {

                if ((modbus->txMessage.data[4] - i) == 1)
                {
                    modbus->txMessage.data[5+i] = (uint8_t) data[i/2];
                    modbus->txMessage.dataSize +=1;
                }
                else
                {
                    modbus->txMessage.data[5+i] = (uint8_t) (data[i/2] >> 8);
                    modbus->txMessage.data[6+i] = (uint8_t) data[i/2];
                    modbus->txMessage.dataSize +=2;
                }
            }
            break;
        }
    }
}

void MTR_Modbus_master_answerChecker(MTR_Modbus_t* modbus)
{
#ifndef CONFIG_MODBUS_DISABLE_SERIAL

    if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection)
    {
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		MTR_Modbus_registerType_t answerType;

		uint8_t* buffer = uartCommunication->callbacksReceiveBuffer->buffer;

		if (buffer[0] != uartCommunication->txBuffer.b.buffer[0]){
			modbus->status = MODBUS_DATAERROR;
			return;
		}

		//Check function
		if (buffer[1] != modbus->txMessage.func)
		{
			modbus->status = MODBUS_DATAERROR;
			// If recieved error functional code (0x80 + fc)
			if (buffer[1] == 0x80 + modbus->txMessage.func){

				modbus->status = MODBUS_STANDARTERROR;
				modbus->lastError = buffer[2];
			}
			return;
		}

		uint16_t crc;
		uint16_t dataQuantity;

		//Check next bytes
		switch (modbus->txMessage.func){

			case  MODBUS_FUNC_READ_COILS_STATUS:

				// Request use bites, answer use bytes. So need to transform bites to bytes
				if (buffer[2]  !=  bitesToBytes((modbus->txMessage.data[2] << 8) + modbus->txMessage.data[3])){
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate package size without CRC
				dataQuantity = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_NEXT_BYTES + buffer[2];

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, dataQuantity);

				//CRC test
				if ((buffer[dataQuantity] != (uint8_t) crc | (buffer[dataQuantity + 1] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;

			case MODBUS_FUNC_READ_DISCR_INPUTS:

				// Request use bites, answer use bytes. So need to transform bites to bytes
				if (buffer[2]   != bitesToBytes((modbus->txMessage.data[2] << 8) + modbus->txMessage.data[3])) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate package size without CRC
				dataQuantity = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_NEXT_BYTES + buffer[2];

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, dataQuantity);

				//CRC test
				if ((buffer[dataQuantity] != (uint8_t) crc | (buffer[dataQuantity + 1] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;

			case  MODBUS_FUNC_READ_HOLDING_REGS:

				//Transform register size to bytes size next
				if (buffer[2]   != (uint8_t) ( modbus->txMessage.data[3]) * 2) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate package size without CRC
				dataQuantity = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_NEXT_BYTES + buffer[2];

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, dataQuantity);

				//CRC test
				if ((buffer[dataQuantity] != (uint8_t) crc | (buffer[dataQuantity + 1] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;


			case MODBUS_FUNC_READ_INPUT_REGS:

				//Transform register size to bytes size next
				if (buffer[2]   != (uint8_t) ( modbus->txMessage.data[3]) * 2) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate package size without CRC
				dataQuantity = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_NEXT_BYTES + buffer[2];

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, dataQuantity);

				//CRC test
				if ((buffer[dataQuantity] != (uint8_t) crc | (buffer[dataQuantity + 1] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;

			case MODBUS_FUNC_WRITE_SINGLE_COIL:

				// Check answer data and request data
				if (buffer[2]   !=  *(&modbus->txMessage.data[0])) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[3]   !=  modbus->txMessage.data[1]) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[4]   !=  modbus->txMessage.data[2]) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[5]   !=  modbus->txMessage.data[3]) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate package size without CRC
				dataQuantity = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY;

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, dataQuantity);

				//CRC test
				if ((buffer[6] != (uint8_t) crc | (buffer[7] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;

			case MODBUS_FUNC_WRITE_SINGLE_REG:

				// Check answer data and request data
				if (buffer[2] != *(&modbus->txMessage.data[0])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[3] != modbus->txMessage.data[1]) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[4] != modbus->txMessage.data[2]) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[5] != modbus->txMessage.data[3]) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate package size without CRC
				dataQuantity = MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY;

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, dataQuantity);

				//CRC test
				if ((buffer[6] != (uint8_t) crc | (buffer[7] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;

			case MODBUS_FUNC_WRITE_MULTIPLE_COILS:

				// Check start register(coil) address
				if ((buffer[2] != modbus->txMessage.data[0]) && (buffer [3] != modbus->txMessage.data[1])) {
					modbus->status = MODBUS_DATAERROR;
					return;

				}

				// Check register(coils) size
				if ((buffer[4] != modbus->txMessage.data[2]) && (buffer [5] != modbus->txMessage.data[3])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY);

				//CRC test
				if ((buffer[6] != (uint8_t) crc | (buffer[7] != (uint8_t) (crc >> 8)))){

					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;


			case MODBUS_FUNC_WRITE_MULTIPLE_REGS:

				// Check start register(coil) address
				if ((buffer[2]   != modbus->txMessage.data[0]) && (buffer [3] != modbus->txMessage.data[1])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				// Check register(coils) size
				if ((buffer[4]   != modbus->txMessage.data[2]) && (buffer [5] != modbus->txMessage.data[3])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				//Calculate CRC
				crc = MTR_Crc_getCrc16Modbus(buffer, MODBUS_BYTE_SIZE_ADDRESS + MODBUS_BYTE_SIZE_FUNCTIONAL_CODE + MODBUS_BYTE_SIZE_START_REGISTER + MODBUS_BYTE_SIZE_QUANTITY);

				//CRC test
				if ((buffer[6] != (uint8_t) crc | (buffer[7] != (uint8_t) (crc >> 8)))){
					modbus->status = MODBUS_OTHERERROR;
					return;
				}

				break;
		}

		modbus->status = MODBUS_CONFIRM;
    }

#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET

    if (modbus->connection == Ethernet)
    {

		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);
		MTR_Modbus_registerType_t answerType;

		MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();

		uint8_t* buffer = tcpData->data;

		if (buffer[0] != (uint8_t)(modbus->txMessage.tid >> 8) || buffer[1] != (uint8_t)(modbus->txMessage.tid & 0x00FF))
		{
			modbus->status = MODBUS_DATAERROR;
			return;
		}

		if (buffer[2] != (uint8_t)(modbus->txMessage.pid >> 8) || buffer[3] != (uint8_t)(modbus->txMessage.pid & 0x00FF))
		{
			modbus->status = MODBUS_DATAERROR;
			return;
		}

		uint16_t recievedLength = (uint16_t)(buffer[4] << 8) + (uint16_t)(buffer[5] & 0x00FF);

		if (recievedLength != tcpData->size - MODBUS_TCP_HEADER_SIZE)
		{
			modbus->status = MODBUS_DATAERROR;
			return;
		}

		if (buffer[6] != modbus->txMessage.address){
			modbus->status = MODBUS_DATAERROR;
			return;
		}

		//Check function
		if (buffer[7] != modbus->txMessage.func)
		{
			modbus->status = MODBUS_DATAERROR;
			// If recieved error functional code (0x80 + fc)
			if (buffer[7] == 0x80 + modbus->txMessage.func){

				modbus->status = MODBUS_STANDARTERROR;
				modbus->lastError = buffer[8];
			}
			return;
		}

		// [2 + MODBUS_TCP_HEADER_SIZE] = [8]

		//Check next bytes
		switch (modbus->txMessage.func){

			case  MODBUS_FUNC_READ_COILS_STATUS:

				// Request use bites, answer use bytes. So need to transform bites to bytes
				if (buffer[8]  !=  bitesToBytes((modbus->txMessage.data[2] << 8) + modbus->txMessage.data[3])){
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;

			case MODBUS_FUNC_READ_DISCR_INPUTS:

				// Request use bites, answer use bytes. So need to transform bites to bytes
				if (buffer[8]   != bitesToBytes((modbus->txMessage.data[2] << 8) + modbus->txMessage.data[3])) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;

			case  MODBUS_FUNC_READ_HOLDING_REGS:

				//Transform register size to bytes size next
				if (buffer[8]   != (uint8_t) ( modbus->txMessage.data[3]) * 2) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;


			case MODBUS_FUNC_READ_INPUT_REGS:

				//Transform register size to bytes size next
				if (buffer[8]   != (uint8_t) ( modbus->txMessage.data[3]) * 2) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;

			case MODBUS_FUNC_WRITE_SINGLE_COIL:

				// Check answer data and request data
				if (buffer[8]   !=  *(&modbus->txMessage.data[0])) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[9]   !=  modbus->txMessage.data[1]) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[10]   !=  modbus->txMessage.data[2]) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[11]   !=  modbus->txMessage.data[3]) {
					//data error
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;

			case MODBUS_FUNC_WRITE_SINGLE_REG:

				// Check answer data and request data
				if (buffer[8] != *(&modbus->txMessage.data[0])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[9] != modbus->txMessage.data[1]) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[10] != modbus->txMessage.data[2]) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				if (buffer[11] != modbus->txMessage.data[3]) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;

			case MODBUS_FUNC_WRITE_MULTIPLE_COILS:

				// Check start register(coil) address
				if ((buffer[8] != modbus->txMessage.data[0]) && (buffer [9] != modbus->txMessage.data[1])) {
					modbus->status = MODBUS_DATAERROR;
					return;

				}

				// Check register(coils) size
				if ((buffer[10] != modbus->txMessage.data[2]) && (buffer [11] != modbus->txMessage.data[3])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;


			case MODBUS_FUNC_WRITE_MULTIPLE_REGS:

				// Check start register(coil) address
				if ((buffer[8]   != modbus->txMessage.data[0]) && (buffer [9] != modbus->txMessage.data[1])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				// Check register(coils) size
				if ((buffer[10]   != modbus->txMessage.data[2]) && (buffer [11] != modbus->txMessage.data[3])) {
					modbus->status = MODBUS_DATAERROR;
					return;
				}

				break;
		}

		modbus->status = MODBUS_CONFIRM;
    }
#endif

}


void MTR_Modbus_master_updateRegistersByReply(MTR_Modbus_t* modbus){

    if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection || modbus->connection == Ethernet){

		MTR_Modbus_registerType_t answerType = (modbus->rxMessage.func == MODBUS_FUNC_READ_INPUT_REGS) ? INPUT : HOLDING;

		if (modbus->rxMessage.func == MODBUS_FUNC_READ_INPUT_REGS || modbus->rxMessage.func == MODBUS_FUNC_READ_HOLDING_REGS){

			for (uint16_t i = 0; i < (uint16_t) (modbus->rxMessage.data[0] / 2); i += 1){
				uint16_t * registerData = MTR_Modbus_master_getPollingRegister(modbus, modbus->txMessage.address, answerType, get16From8Bits(modbus->txMessage.data) + i);
				*(registerData) = get16From8Bits(&modbus->rxMessage.data[1 + 2*i]);
			}
		}
    }
}


void MTR_Modbus_master_requestHandler(MTR_Modbus_t* modbus){

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection || modbus->connection == Uart2Connection){

		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(modbus->connection);

		MTR_Modbus_master_specificData* specificData = modbus->specificModbusData;

		MTR_Modbus_device_t* deviceStatus = MTR_Modbus_master_getDevice(modbus, modbus->txMessage.address);

		if (modbus->status == MODBUS_DISPATCH){

			MTR_Uart_BufferStruct_t* buffer = uartCommunication->callbacksReceiveBuffer;

			if (buffer->amount == 0)
			{
				specificData->totalTxNoAnswerPackages += 1;
				deviceStatus->noAnswerErrorCounter += 1;
				deviceStatus->lastRequestHasReply = false;

				if (deviceStatus->statusCounter > 0){
					deviceStatus->statusCounter = deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE ? deviceStatus->statusCounter - CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE : 0;
				}
			}
			else
			{

				if (buffer->amount >= MODBUS_RTU_PACKET_SIZE_MIN && buffer->buffer[0] == modbus->txMessage.address){

					modbus->totalRxPackages +=1;
					deviceStatus->rxCounter += 1;

					modbus->status = MODBUS_RECEIVED;
					modbus->lastError = MODBUS_ERROR_NONE;

					MTR_Modbus_master_answerChecker(modbus);

					if (modbus->status == MODBUS_CONFIRM){

						MTR_Modbus_master_callbackAfterProccess(modbus);

						deviceStatus->lastRequestHasReply = true;

						modbus->totalRxGoodPackages += 1;
						deviceStatus->rxGoodCounter += 1;

						modbus->rxMessage.address = buffer->buffer[0];
						modbus->rxMessage.func = buffer->buffer[1];

						memcpy(modbus->rxMessage.data, buffer->buffer + 2, buffer->amount - 4);
						modbus->rxMessage.dataSize = buffer->amount - 4;
						modbus->rxMessage.crc = buffer->buffer[buffer->amount - 2] << 8 || buffer->buffer[buffer->amount - 1];

						MTR_Modbus_master_updateRegistersByReply(modbus);

						if (deviceStatus->statusCounter < CONFIG_MODBUS_RTU_MASTER_RECENTY_GOOD_MESSAGES_AS_ONLINE * 2){
							deviceStatus->statusCounter += 1;
						}
					}
					else
					{
						//something was recieved, but error occured
						modbus->totalRxErrorPackages +=1;
						deviceStatus->otherError += 1;

						if (deviceStatus->statusCounter > 0){
							deviceStatus->statusCounter = deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE ? deviceStatus->statusCounter - CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE : 0;
						}
					}
				}
				else
				{
					modbus->totalRxErrorPackages += 1;
					deviceStatus->recieveError += 1;
					deviceStatus->lastRequestHasReply = false;

					if (deviceStatus->statusCounter > 0){
						deviceStatus->statusCounter = deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE ? deviceStatus->statusCounter - CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE : 0;
					}
				}
			}
		}



		deviceStatus->online = (deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_RECENTY_GOOD_MESSAGES_AS_ONLINE);

		MTR_Modbus_resetRxBuffer(modbus);
		MTR_Modbus_resetTxBuffer(modbus);
		modbus->status = MODBUS_READY;
	}

#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (modbus->connection == Ethernet)
	{
		MTR_Modbus_master_specificData* specificData = modbus->specificModbusData;

		MTR_Modbus_device_t* deviceStatus = MTR_Modbus_master_getDevice(modbus, modbus->txMessage.address);

		MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();

		if (modbus->status == MODBUS_DISPATCH){

			if (tcpData->size == 0)
			{
				specificData->totalTxNoAnswerPackages += 1;
				deviceStatus->noAnswerErrorCounter += 1;
				deviceStatus->lastRequestHasReply = false;

				if (deviceStatus->statusCounter > 0){
					deviceStatus->statusCounter = deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE ? deviceStatus->statusCounter - CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE : 0;
				}
			}
			else
			{
				if (tcpData->size >= MODBUS_RTU_PACKET_SIZE_MIN && tcpData->data[MODBUS_TCP_HEADER_SIZE] == modbus->txMessage.address){

					modbus->totalRxPackages +=1;
					deviceStatus->rxCounter += 1;

					modbus->status = MODBUS_RECEIVED;
					modbus->lastError = MODBUS_ERROR_NONE;

					MTR_Modbus_master_answerChecker(modbus);

					if (modbus->status == MODBUS_CONFIRM){

						deviceStatus->lastRequestHasReply = true;

						modbus->totalRxGoodPackages += 1;
						deviceStatus->rxGoodCounter += 1;

						modbus->rxMessage.tid |= (uint16_t)(tcpData->data[0] << 8);
						modbus->rxMessage.tid |= (uint16_t) tcpData->data[1];
						modbus->rxMessage.pid |= (uint16_t)(tcpData->data[2] << 8);
						modbus->rxMessage.pid |= (uint16_t) tcpData->data[3];
						modbus->rxMessage.length |= (uint16_t)(tcpData->data[4] << 8);
						modbus->rxMessage.length |= (uint16_t) tcpData->data[5];

						modbus->rxMessage.address = tcpData->data[6];
						modbus->rxMessage.func = tcpData->data[7];

						memcpy(modbus->rxMessage.data, tcpData->data + 8, tcpData->size - 8);
						modbus->rxMessage.dataSize = tcpData->size - 8;

						MTR_Modbus_master_updateRegistersByReply(modbus);

						if (deviceStatus->statusCounter < CONFIG_MODBUS_RTU_MASTER_RECENTY_GOOD_MESSAGES_AS_ONLINE * 2){
							deviceStatus->statusCounter += 1;
						}
					}
					else
					{
						//something was recieved, but error occured
						modbus->totalRxErrorPackages +=1;
						deviceStatus->otherError += 1;

						if (deviceStatus->statusCounter > 0){
							deviceStatus->statusCounter = deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE ? deviceStatus->statusCounter - CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE : 0;
						}
					}
				}
				else
				{
					modbus->totalRxErrorPackages += 1;
					deviceStatus->recieveError += 1;
					deviceStatus->lastRequestHasReply = false;

					if (deviceStatus->statusCounter > 0){
						deviceStatus->statusCounter = deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE ? deviceStatus->statusCounter - CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE : 0;
					}
				}
			}
		}

		deviceStatus->online = (deviceStatus->statusCounter >= CONFIG_MODBUS_RTU_MASTER_RECENTY_GOOD_MESSAGES_AS_ONLINE);

		MTR_Modbus_resetRxBuffer(modbus);
		MTR_Modbus_resetTxBuffer(modbus);
		modbus->status = MODBUS_READY;
	}
#endif
}

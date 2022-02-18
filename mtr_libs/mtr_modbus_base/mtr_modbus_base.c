#include "mtr_modbus_base/mtr_modbus_base.h"

//! Start modbus element of list
MTR_Modbus_modbusElement* startModbus;

MTR_Modbus_modbusElement* MTR_Modbus_startModbus()
{
	return startModbus;
}

bool_t MTR_Modbus_setStartModbus(MTR_Modbus_modbusElement* element)
{
	if (!startModbus){
		startModbus = element;
		return true;
	}

	return false;
}

void MTR_Modbus_resetTxBuffer(MTR_Modbus_t* modbus) {
	toZeroByteFunction((uint8_t*) &modbus->txMessage, sizeof(MTR_Modbus_message_t));
}

void MTR_Modbus_resetRxBuffer(MTR_Modbus_t* modbus) {
#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (modbus->connection == Uart1Connection ||
			modbus->connection == Uart2Connection)
	{
		MTR_Uart_resetRxPointer(modbus->connection, 0);
	}

#endif
	modbus->status = MODBUS_READY;
}

MTR_Modbus_t* MTR_Modbus_getModbusStruct(AvaliableConnectionType_t connection, uint8_t slaveAddress)
{
	MTR_Modbus_modbusElement* modbusListElement = startModbus;
	//counter for loop
	uint8_t counter = 255;

	if (startModbus != 0) {
		for(;;){

			if (modbusListElement->modbus->connection == connection && modbusListElement->modbus->address == slaveAddress) {
				break;
			}

			if (modbusListElement->nextModbusElement == 0) {
				return 0;
			}

			//loop counter
			if (counter != 0)
			{
				counter--;
				if (counter == 0){
					return 0;
				}
			}

			modbusListElement = modbusListElement->nextModbusElement;
		}

		return modbusListElement->modbus;
	}

	return 0;
}

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
void MTR_ModbusRtu_uartSwitchToRecieve(AvaliableConnectionType_t uart)
{
    MTR_Modbus_t* modbus = MTR_Modbus_getModbusStruct(uart, 0);

	if (modbus) {
		//check for master, if master, refresh recieve timeout of uart struct
		if (modbus->mode == MODBUS_MODE_MASTER){
			MTR_Uart_timerRefresh(uart, uartTimerRepeatesUserTimeout1);
		}
	}
}

#endif

void __attribute__((weak)) MTR_Modbus_master_requestHandler(MTR_Modbus_t* modbusMaster);
void __attribute__((weak)) MTR_Modbus_slave_requestHandler(MTR_Modbus_t* modbusMaster);

void MTR_Modbus_performHandler(AvaliableConnectionType_t connection)
{
	MTR_Modbus_t* modbusMaster = 0;

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
	if (connection == Uart1Connection || connection == Uart2Connection)
	{
		Uart_CommunicationStruct_t* uartCommunication = MTR_Uart_getCommunicationStruct(connection);
		// 0 its modbus master
		modbusMaster = MTR_Modbus_getModbusStruct(connection, 0);

		//try to found modbus master, if didnt, its slave
		if (modbusMaster) {
			MTR_Modbus_master_requestHandler(modbusMaster);
		}
		else
		{
			MTR_Modbus_t* modbus = MTR_Modbus_getModbusStruct(connection, uartCommunication->callbacksReceiveBuffer->buffer[0]);

			if (modbus) {
				MTR_Modbus_slave_requestHandler(modbus);
			}
		}

		MTR_Uart_resetRxPointer(connection, 0);
	}
#endif
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	if (connection == Ethernet)
		{
		MTR_Tcp_data_t* tcpData = MTR_Tcp_getReceivedData();
		// 0 its modbus master
		modbusMaster = MTR_Modbus_getModbusStruct(connection, 0);

		//try to found modbus master, if didnt, its slave
		if (modbusMaster) {
			MTR_Modbus_master_requestHandler(modbusMaster);
		}
		else
		{
			MTR_Modbus_t* modbus = MTR_Modbus_getModbusStruct(connection,
					tcpData->data[MODBUS_TCP_HEADER_SIZE]);

			if (modbus) {
				MTR_Modbus_slave_requestHandler(modbus);
			}
		}
	}

#endif
}

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
void MTR_Modbus_Ethernet_performHandler() {
	 MTR_Modbus_performHandler(Ethernet);
}
#endif


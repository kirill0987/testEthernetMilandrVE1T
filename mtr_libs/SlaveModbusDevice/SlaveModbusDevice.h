#ifndef CLASSES_DATAMODEL_TEMPLATEMODBUSDEVICE_H_
#define CLASSES_DATAMODEL_TEMPLATEMODBUSDEVICE_H_

#include "BaseNetworkDevices/ModbusDevice.h"
#include <array>

template< 	uint8_t amountInputRegs,	\
			uint8_t amountHoldingRegs >
struct SlaveModbusDeviceData
{
	std::array<uint16_t*, amountInputRegs> inputRegisters;
	std::array<uint16_t*, amountHoldingRegs> holdingRegisters;
};

template< 	uint8_t amountInputRegs,						\
			uint8_t amountHoldingRegs,						\
			typename dataStruct =							\
					SlaveModbusDeviceData< amountInputRegs,	\
					  	  	  	  	  	   amountHoldingRegs > >
class SlaveModbusDevice: public ModbusDevice<dataStruct>
{
public:
	SlaveModbusDevice(	uint32_t networkAddress,	\
								MTR_Modbus_t* connection)
	: ModbusDevice<dataStruct>(networkAddress, connection)
	{
		for( uint8_t i = 0; i < amountInputRegs; i++ )
		{
			ModbusDevice<dataStruct>::data()->inputRegisters[i] =	\
					MTR_Modbus_master_getPollingRegister(			\
							connection, 							\
							networkAddress,							\
							INPUT,									\
							i );
		}
	}

	void pullInputRegisters(			\
			uint16_t startRegister = 0,	\
			uint8_t quantity = amountInputRegs )
	{
		MTR_Modbus_master_getInputRegisters(				\
				ModbusDevice<dataStruct>::_mb,				\
				ModbusDevice<dataStruct>:: _networkAddress,	\
				startRegister,								\
				quantity );
	}

	void pullHoldingRegisters(			\
			uint16_t startRegister = 0,	\
			uint8_t quantity = amountHoldingRegs )
	{
		MTR_Modbus_master_getHoldingRegisters(				\
				ModbusDevice<dataStruct>::_mb,				\
				ModbusDevice<dataStruct>:: _networkAddress,	\
				startRegister,								\
				quantity );
	}
};

#endif /* CLASSES_DATAMODEL_TEMPLATEMODBUSDEVICE_H_ */

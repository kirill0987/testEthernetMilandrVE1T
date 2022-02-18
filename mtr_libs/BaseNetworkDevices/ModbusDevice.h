#ifndef CLASSES_MODBUSDEVICE_H_
#define CLASSES_MODBUSDEVICE_H_
#include "NetworkDevice.h"

#include "mtr_libs/mtr_modbus/mtr_modbus.h"

template <typename T, typename... Args>
class ModbusDevice : public NetworkDevice<T, Args...>{
public:
	ModbusDevice(
			uint32_t networkAddress,
			MTR_Modbus_t* connection,
			Args... args
			)
		: NetworkDevice<T, Args...>(networkAddress, connection->connection, args...),
		  _mb{connection}
	{
		_mbDevice = MTR_Modbus_master_getDevice(connection, (uint8_t) networkAddress);
	}

	MTR_Modbus_device_t * mbDevice(){
		return _mbDevice;
	}

	MTR_Modbus_t* modbus() {return _mb;}

	void updateOnline(){
		//NetworkDevice::_isOnline = _mbDevice->online;
	}

protected:
	MTR_Modbus_device_t* _mbDevice;
	MTR_Modbus_t* _mb;

};

#endif /* CLASSES_MODBUSDEVICE_H_ */

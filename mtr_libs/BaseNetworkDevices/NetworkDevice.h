#ifndef CLASSES_NETWORKDEVICE_H_
#define CLASSES_NETWORKDEVICE_H_
#include <stdint.h>
#include "mtr_service/mtr_service.h"

template <typename T, typename... Args>
class NetworkDevice {
public:
	NetworkDevice(uint32_t networkAddress, AvaliableConnectionType_t connection, Args... args) :
		_isOnline{false},
		_networkAddress{networkAddress},
		_networkConnection{connection},
		_data{new T(args...)}
		{

		};

	uint32_t networkAddress() const {
		return _networkAddress;
	}

	AvaliableConnectionType_t networkConnection() const{
		return _networkConnection;
	}

	void updateOnline(){};

	bool isOnline() const {
		return _isOnline;
	}

	T* data() const{
		return _data;
	}

protected:
	bool _isOnline;
	uint32_t _networkAddress;
	AvaliableConnectionType_t _networkConnection;

	T * _data;

};

#endif /* CLASSES_NETWORKDEVICE_H_ */

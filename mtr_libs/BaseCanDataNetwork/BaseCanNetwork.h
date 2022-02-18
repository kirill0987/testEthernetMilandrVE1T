#ifndef CLASSES_BASECANNETWORK_H_
#define CLASSES_BASECANNETWORK_H_

#include <stdint.h>
#include "mtr_can/mtr_can.h"
#include "BaseDataManager/BaseDataManager.h"

class BaseCanNetwork {

public:
	BaseCanNetwork(uint8_t unitId, BaseDataManager * dataManager);

	uint8_t unitId() const;

	void sendInputsAmount();
	void sendOutputsAmount();

	void checkSingleControlMessage(AvaliableConnectionType_t connection, CAN_RxMsgTypeDef const & RxMessage);
	void sendErrorMessage(MTR_Can_Name_t can, uint8_t errorDevice, uint8_t code);

	void sendLifeFrame(MTR_Can_Name_t connection);

private:
	uint8_t _unitId;
	BaseDataManager * _dataManager;

};

#endif /* CLASSES_BASECANNETWORK_H_ */

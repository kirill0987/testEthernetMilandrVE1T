#ifndef CLASSES_CANNETWORKING_H_
#define CLASSES_CANNETWORKING_H_

#include "BaseCanDataNetwork/BaseCanNetwork.h"
#include "mtr_libs/mtr_planner/mtr_planner.h"

class DataManager;

class CanNetworking : public BaseCanNetwork
{
public:
	//constructor
	CanNetworking(uint8_t unitId, DataManager* dataManager);

	//init
	void initCan1();
	void initCan2();

	//handler
	void canHandler();
	void canRecieve(uint8_t i, CAN_RxMsgTypeDef* rxMessage);

	//Frame
	void initTestCAN1();
	void initTestCAN2();

	void initFrameServoCan1();

private:
	uint32_t _temporaryCanData[2];
	DataManager* _dataManager;
};

#endif /* CLASSES_CANNETWORKING_H_ */

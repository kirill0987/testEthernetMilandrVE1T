#include "BaseCanNetwork.h"
#include "dataenums.h"
#include "mtr_libs/mtr_can/mtr_can.h"

#include <vector>

const uint8_t readDiCanId = 0x00;
const uint8_t readDoCanId = 0x01;
const uint8_t writeDoCanId = 0x02;
const uint8_t readValueCanId = 0x03;
const uint8_t writeValueCanId = 0x04;
const uint8_t startStopCommandCanId = 0xff;

BaseCanNetwork::BaseCanNetwork(uint8_t unitId, BaseDataManager * dataManager) :
		_unitId{unitId}
		, _dataManager{dataManager}
{

}


void BaseCanNetwork::sendInputsAmount(){

	uint32_t tempCanData[2];

	tempCanData[0] = (uint16_t) AllBoolInputs::DiEnd << 16;

	MTR_Can_sendExtendedMessage(CAN1, 0x00010001, 2, (uint8_t *) tempCanData);
}

void BaseCanNetwork::sendOutputsAmount(){

	uint32_t tempCanData[2];

	tempCanData[0] = (uint16_t) AllBoolOutputs::DoEnd << 16;

	MTR_Can_sendExtendedMessage(CAN1, 0x00010001, 2, (uint8_t *) tempCanData);
}


void BaseCanNetwork::checkSingleControlMessage(	AvaliableConnectionType_t connection,
												CAN_RxMsgTypeDef const & RxMessage)
{
	if ((uint8_t)(RxMessage.Rx_Header.ID >> 8) != _unitId){
		return;
	}

	switch (((uint8_t)(RxMessage.Rx_Header.ID >> 16))) {
		case readDiCanId:
		case readDoCanId:
		case writeDoCanId:
		case readValueCanId:
		case writeValueCanId:
		case startStopCommandCanId:
			break;
		default:
			return;
	}


	std::vector<uint8_t> data;
	data.reserve(8);

	uint8_t bitMask = 0;
	uint8_t commandId = (RxMessage.Rx_Header.ID >> 16);

	switch (commandId) {
		case readDiCanId:
		{
			if (RxMessage.Rx_Header.DLC == 2){
				//read di
				uint16_t diId = (uint16_t) (swapBytes32(RxMessage.Data[0]) >> 16);
				int i = 0;

				if (diId < AllBoolInputs::DiEnd){
					data.push_back(diId >> 8);
					data.push_back((uint8_t) diId);

					bitMask |= (bool)((*_dataManager)[static_cast<AllBoolInputs>(diId)]) << i++;
				}

				data.push_back(bitMask);
			}
		}
			break;
		case readDoCanId:
			if (RxMessage.Rx_Header.DLC == 2){
				//read do
				uint16_t doId = (uint16_t) (swapBytes32(RxMessage.Data[0]) >> 16);
				int i = 0;

				if (doId < AllBoolOutputs::DoEnd){
					data.push_back(doId >> 8);
					data.push_back((uint8_t) doId);

					bitMask |= (bool)((*_dataManager)[static_cast<AllBoolOutputs>(doId)]) << i++;
				}

				data.push_back(bitMask);
			}

			break;
		case writeDoCanId:
			if (RxMessage.Rx_Header.DLC == 3){
				//read do
				uint16_t doId = (uint16_t) (RxMessage.Data[0] >> 16);
				bool state = getBit(RxMessage.Data[0], 8);

				int i = 0;

				if (doId < AllBoolOutputs::DoEnd){
					data.push_back(doId >> 8);
					data.push_back((uint8_t) doId);

					(*_dataManager)[static_cast<AllBoolOutputs>(doId)] = state;

					bitMask |= (bool)((*_dataManager)[static_cast<AllBoolOutputs>(doId)]) << i++;
				}

				data.push_back(bitMask);
			}

			break;

		case startStopCommandCanId:
			if (RxMessage.Rx_Header.DLC == 1){
				uint8_t state = (uint8_t) (RxMessage.Data[0] >> 24);

				if (state == 0 && _dataManager->started()){
					_dataManager->stopOperating();
				}
				else if (state == 1 && !_dataManager->started()){
					_dataManager->startOperating();
				}
				else {
					state = 0xff;
				}

				data.push_back(state);
			}
			break;

		case readValueCanId:

			if (RxMessage.Rx_Header.DLC == 2){
				//read do
				uint16_t id = (uint16_t) (swapBytes32(RxMessage.Data[0]) >> 16);

				if (id < _dataManager->outputData().size()){
					data.push_back(id >> 8);
					data.push_back((uint8_t) id);

					//uint32_t value = (uint32_t) _dataManager->outputData().at(id)->value();

//					data.push_back(value >> 24);
//					data.push_back(value >> 16);
//					data.push_back(value >> 8);
//					data.push_back(value);
				}

				data.push_back(bitMask);
			}

			break;

		case writeValueCanId:

			if (RxMessage.Rx_Header.DLC >= 2){
				//read do
				uint16_t id = (uint16_t) (swapBytes32(RxMessage.Data[0]) >> 16);

				if (id < _dataManager->outputData().size()){
					data.push_back(id >> 8);
					data.push_back((uint8_t) id);

					//int targetValueSize = _dataManager->outputData().at(id)->size();

//					if (targetValueSize == 2 + targetValueSize){
//
//						uint32_t value = (uint32_t) (swapBytes32(RxMessage.Data[0]) << 16) | \
//								(swapBytes32(RxMessage.Data[1]) >> 16);
//
//						for (int i = 4; i < 0; i--) {
//							if (i == targetValueSize) {
//								break;
//							}
//
//							value &= ~(0xFF000000 >> (8 * (4 - i)));
//						}
//
//						for (int i = 0; i < targetValueSize; i++) {
//							data.push_back(value >> 8);
//						}
//					}
				}

				data.push_back(bitMask);
			}

			break;
	}

	MTR_Can_sendExtendedMessageDirect((MTR_Can_Name_t)connection, (commandId + 0x10) << 16 | (RxMessage.Rx_Header.ID & 0xFF00FFFF), data.size(), data.data());
}


void BaseCanNetwork::sendLifeFrame(MTR_Can_Name_t connection){
	MTR_Can_sendExtendedMessageDirect(connection, _unitId, 0, nullptr);
}

void BaseCanNetwork::sendErrorMessage(MTR_Can_Name_t can, uint8_t errorDevice, uint8_t code){
	uint16_t temp = code << 8 | errorDevice;
	MTR_Can_sendExtendedMessageDirect(can, 0x0FFF0000 | (_unitId << 8), 2, (uint8_t*) &temp);
}


uint8_t BaseCanNetwork::unitId() const{
	return _unitId;
}

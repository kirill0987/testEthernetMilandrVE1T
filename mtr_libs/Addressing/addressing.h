#ifndef ADDRESSING_ADDRESSING_H_
#define ADDRESSING_ADDRESSING_H_

#include "mtr_libs/mtr_modbus/mtr_modbus.h"
#include "mtr_libs/mtr_planner/mtr_planner.h"

#define allowedWorkThisModule 0
#define notAllowedWorkThisModule 1

const uint8_t slaveDefaultAddress = 247;

#ifdef __cplusplus
extern "C" {
#endif


//0 - Включен режим прошивки адресов
//1 - Адрес выбран
//2 - Ведомый ответил без адреса
//3 - Адрес был назначен
//4 - Адрес не был назначен
//5 - Моргание светодиода
typedef enum
{
	AddressingModeOn_Addressing,				//0
	AddressSelected_Addressing,					//1
	SlaveRespondedWithoutAddress_Addressing,	//2
	AddressWasAssigned_Addressing,				//3
	AddressWasNotAssigned_Addressing,			//4
	BlinkLED_Addressing							//5
}AddressingEvents_t;

//0 - Кнопка нажата и удерживается, значит идет режим прошивки адреса
//1 - Отжата кнопка значит завершён режим прошивки адресов
//2 - По нажатию кнопки инкрементируется адрес
//3 - Отжата кнопка (не интересно)
//4 - Нажата кнопка у ведомго и идет feedback от slave (идет запись адреса)
//5 - Отжата кнопка у slave (базово адрес прошит, но может и нет)
typedef enum
{
	ButtonPressed_AssignmentModeMaster_Addressing,
	ButtonReleased_AssignmentModeMaster_Addressing,

	ButtonPressed_IncrementAddressSlaveForAssignmentModeMaster_Addressing,
	ButtonReleased_IncrementAddressSlaveForAssignmentModeMaster_Addressing,

	ButtonPressed_FeedbackWriteAddressSlave_Addressing,
	ButtonReleased_FeedbackWriteAddressSlave_Addressing
}ButtonEvents_t;

typedef enum
{
	StateNone_Addressing,					//Не выбран режим адресации
	StateAddressSelected_Addressing,		//выбран адрес
	StateSlavePressedButton_Addressing,		//Получена обратная связь от ведомого
	StateSlaveReleasedButton_Addressing,	//Кнопка обратной связи отпущена на ведомом
	StateAddressAssigned_Addressing			//Назначен адрес
}AddresingStates_t;

typedef struct
{
	MTR_Modbus_t* modbusInternalRS;			//указатель на modbus внутренней шины
	MTR_Modbus_device_t* slaveDevice;		//указатель устройства
	AddresingStates_t addressingState;		//этап назначения адреса
	uint8_t addressToAssign;				//адрес, назначаемый ведомому
	bool_t addressAssignmentMode;			//включен режим записи адреса
	bool_t addressWasAssigned;				//адрес был назначен в текущей процедуре адресации
	bool_t addressWasAssignedBefore;		//адрес был назначен ранее
	bool_t addressingReset;					//ведомый нажал кнопку без выбора адреса,
											//значит адресация производится с другого ведущего

	int stageInternalRS;					//этап обмена по modbus
}BaseAddressing_t;

void Addressing_initData(MTR_Modbus_t* modbus);
void Addressing_modbusPoll();
void Addressing_buttonHandler(ButtonEvents_t buttonEvent);
void Addressing_resetCounterDevice();
bool_t Addressing_getPreviousAssignment();
bool_t Addressing_getAssignmentMode();

#ifdef __cplusplus
}
#endif

#endif /* ADDRESSING_ADDRESSING_H_ */

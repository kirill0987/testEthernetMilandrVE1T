#ifndef CLASSES_DATAMODEL_SERVO_H_
#define CLASSES_DATAMODEL_SERVO_H_

#include "Servo_defines.h"
#include <stdint.h>
#include <string.h>

//Шаги запроса и передачи данных
typedef enum
{
	SpeedContour_SSFC,
	Position_SSFC,
	StateControoler_SSFC,
	StateDI_SSFC,
	RPM_SSFC,
	Current_SSFC,

	SettingSpeedContour_SSFC,
	SettingRPM_SSFC,
	SettingCurrent_SSFC
}ServoStageFrameCAN_t;

//Состояние контура скорости
typedef enum
{
	Current_UnlockContour_SCSS = 0,	//контур разомкнут - можно управлять током, но нельзя скоростью
	RPM_LockContour_SCSS = 2		//контур замкнут - можно управлять скоростью, но нельзя током
}ServoSpeedContourState_t;

//Обратная связь от сервопривода
struct FeedBackServo_t
{
	float rpm;
	float current;
	ServoSpeedContourState_t speedContour;
	int32_t position;
	uint16_t stateControolerServo;
	uint8_t DI;
};

//Уставка сервопирвода
struct SettingServo_t
{
	float rpm;
	float current;
	ServoSpeedContourState_t speedContour;
};

//Параметры онлайна сервопривода
struct OnlineServo_t
{
	uint8_t addressCan;
	bool online : 1;
	uint8_t counterFrame;
};

//Данные сервопривода
struct requestAndSetServo_t
{
	uint32_t ID;

	ServoStageFrameCAN_t stageCAN;
};

//Данные сервопривода
struct DataServo_t
{
	OnlineServo_t online;
	SettingServo_t setting;
	FeedBackServo_t feedBack;

};

class Servo
{
public:
	Servo();

	//control
	void controlOnePressServo();
	void controlOnePusherServo();

	//converts
	uint32_t convertRPM_forWrite(float rpm);
	float convertRPM_forRead(uint32_t rpm);

	uint32_t convertCurrent_forWrite(float current);
	float convertCurrent_forRead(uint16_t current);

	//handler
	void requestAndSetHandlerCAN( uint8_t numberServoInStruct );

	void answerHandlerCAN(	uint8_t numberServoInStruct,
							uint16_t paremetr,
							uint32_t data);

	//GETTER
	DataServo_t* getServo(uint8_t number);
private:
	DataServo_t _servos[amountServos];
};

#endif /* CLASSES_DATAMODEL_SERVO_H_ */

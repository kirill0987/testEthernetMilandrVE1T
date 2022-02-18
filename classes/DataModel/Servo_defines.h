
#ifndef CLASSES_DATAMODEL_SERVO_DEFINES_H_
#define CLASSES_DATAMODEL_SERVO_DEFINES_H_

#define amountServos 14

#define amountServosCAN1 7
#define amountServosCAN2 7

#define addressSourceMessage						0x0

#define commandRequestTechnologicalProtorol			0xD //13 - work only this in CAN
#define commandAnswerTechnologicalProtorol			0xE //14 - work only this in CAN

#define commandRequestParameter						0x5 //5 - запрос
#define commandAnswerParameter						0x6 //6 - ответ
#define commandSetParameter							0x7 //7 - установка

//---Address parameter servo---
//-staticParameter-
//bool 		| false...true
#define speedContourState_vp7		0x0011

//bit 4, 5 in byte D3  - state discrete input
#define statePortInputOutput_ip5	0x001D

//float 	| -4...4 Ampere
#define current_Ct1					0x0200

//-parameter control-
//int16_t 	| -7000...7000 RPM
#define RPM_Ct12					0x0202

//-dynamic parameter-
//int32_t	| -2147483647...2147483647 discrete (Byte in CAN D3...D6)
#define currentPosition_DD8			0x040C

//uint8_t
#define stateController_DD11		0x040F

//For recalculate rpm
#define servo_convertRPM 4096.0

//CAN1 (add 15-21)
#define idAnswerServoCAN_add_15 0x00038780
#define idAnswerServoCAN_add_16 0x00038800
#define idAnswerServoCAN_add_17 0x00038880
#define idAnswerServoCAN_add_18 0x00038900
#define idAnswerServoCAN_add_19 0x00038980
#define idAnswerServoCAN_add_20 0x00038A00
#define idAnswerServoCAN_add_21 0x00038A80

#endif /* CLASSES_DATAMODEL_SERVO_DEFINES_H_ */

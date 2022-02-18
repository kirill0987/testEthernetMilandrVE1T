#include "CanNetworking.h"
#include "mtr_can/mtr_can.h"
#include "classes/DataManager.h"
#include "inc/device.h"
#include "config.h"
#include "string.h"

#include "classes/DataModel/Servo.h"

extern Servo* servo;

CanNetworking::CanNetworking(	uint8_t unitId,
								DataManager* dataManager):
	BaseCanNetwork( unitId, dataManager ),
	_dataManager	{ dataManager }
{
	memset( _temporaryCanData, 0, sizeof( _temporaryCanData ) );
}

void CanNetworking::initTestCAN1()
{
	memset( _temporaryCanData, 0, sizeof( _temporaryCanData ) );

	_temporaryCanData[0] = 	1 << 0 	|
							2 << 8 	|
							3 << 16 |
							4 << 24;

	_temporaryCanData[1] = 	5 << 0 	|
							6 << 8 	|
							7 << 16 |
							8 << 24;

	MTR_Can_sendExtendedMessageDirect(
			CAN1 ,
			0x00000001,
			8,
			(uint8_t*) _temporaryCanData);
}

void CanNetworking::initTestCAN2()
{
	memset( _temporaryCanData, 0, sizeof(_temporaryCanData) );

	_temporaryCanData[0] = 	10 << 0 	|
							12 << 8 	|
							13 << 16 	|
							14 << 24;

	_temporaryCanData[1] = 	15 << 0 	|
							16 << 8 	|
							17 << 16 	|
							18 << 24;

	MTR_Can_sendExtendedMessageDirect(
			CAN2,
			0x00000002,
			8,
			(uint8_t*) _temporaryCanData);
}

void CanNetworking::initFrameServoCan1()
{
	/*
	static uint32_t ID = 0x0;
	static uint32_t lenght = 0x0;
	static uint8_t counterServo = 0;
	DataServo_t* servoStruct = servo->getServo(counterServo);
	ID = 	commandRequestTechnologicalProtorol << 14 	|
			addressSourceMessage 				<< 7 	|
			servoStruct->online.addressCan 		<< 0;
	lenght = 0;

	memset( _temporaryCanData, 0, sizeof(_temporaryCanData) );

	switch( servoStruct->stageCAN )
	{
		case( SpeedContour_SSFC ):
		{
			_temporaryCanData[0] = 	commandRequestParameter 			<< 0 |
									swapBytes16(speedContourState_vp7) 	<< 8;

			lenght = 3;

			servoStruct->stageCAN = Position_SSFC;
		}
		break;

		case( Position_SSFC ):
		{
			_temporaryCanData[0] = 	commandRequestParameter 			<< 0 |
									swapBytes16(currentPosition_DD8) 	<< 8;

			lenght = 3;

			servoStruct->stageCAN = StateControoler_SSFC;
		}
		break;

		case( StateControoler_SSFC ):
		{
			_temporaryCanData[0] = 	commandRequestParameter 			<< 0 |
									swapBytes16(stateController_DD11) 	<< 8;

			lenght = 3;

			servoStruct->stageCAN = StateDI_SSFC;
		}
		break;

		case( StateDI_SSFC ):
		{
			_temporaryCanData[0] = 	commandRequestParameter 				<< 0 |
									swapBytes16(statePortInputOutput_ip5) 	<< 8;

			lenght = 3;

			servoStruct->stageCAN = RPM_SSFC;
		}
		break;

		case( RPM_SSFC ):
		{
			_temporaryCanData[0] = 	commandRequestParameter << 0 |
									swapBytes16(RPM_Ct12) 	<< 8;

			lenght = 3;

			servoStruct->stageCAN = Current_SSFC;
		}
		break;

		case( Current_SSFC ):
		{
			_temporaryCanData[0] = 	commandRequestParameter 	<< 0 |
									swapBytes16(current_Ct1) 	<< 8;

			lenght = 3;

			servoStruct->stageCAN = SettingSpeedContour_SSFC;
		}
		break;

		case( SettingSpeedContour_SSFC ):
		{
			_temporaryCanData[0] = 	commandSetParameter 				<< 0 |
									swapBytes16(speedContourState_vp7) 	<< 8 |
									static_cast<uint8_t>(servoStruct->setting.speedContour)	<< 24;

			lenght = 7;

			servoStruct->stageCAN = SettingRPM_SSFC;
		}
		break;

		case( SettingRPM_SSFC ):
		{
			uint32_t temp = servo->convertRPM_forWrite( servoStruct->setting.rpm );

			_temporaryCanData[0] = 	commandSetParameter 	<< 0 |
									swapBytes16(RPM_Ct12) 	<< 8 |
									( temp & 0xFF )			<< 24;

			_temporaryCanData[1] = ( temp >> 8 );

			lenght = 7;

			servoStruct->stageCAN = SettingCurrent_SSFC;
		}
		break;

		case( SettingCurrent_SSFC ):
		{
			uint32_t temp = servo->convertCurrent_forWrite( servoStruct->setting.current );

			_temporaryCanData[0] = 	commandSetParameter 		<< 0 |
									swapBytes16(current_Ct1) 	<< 8 |
									( temp & 0xFF )				<< 24;

			_temporaryCanData[1] = temp >> 8;

			lenght = 7;

			servoStruct->stageCAN = SpeedContour_SSFC;
		}
		break;

		default:
		{
			servoStruct->stageCAN = SpeedContour_SSFC;
		}
		break;
	}

	( counterServo >= (amountServosCAN1 - 1) ) ?
			counterServo = 0 : counterServo++;

	MTR_Can_sendExtendedMessageDirect(
			CAN1,
			ID,
			lenght,
			(uint8_t*) _temporaryCanData);
			*/
}

void CanNetworking::canRecieve(uint8_t i, CAN_RxMsgTypeDef* rxMessage)
{
	//!Datasheet!
	if (rxMessage->Rx_Header.IDE == 0){
		rxMessage->Rx_Header.ID = rxMessage->Rx_Header.ID >> 18;
	}

	//Специально добавил чтобы можно было видеть как в CANwise
	rxMessage->Data[0] = swapBytes32(rxMessage->Data[0]);
	rxMessage->Data[1] = swapBytes32(rxMessage->Data[1]);

	uint8_t tempCommandTechnologicalProtorol =
			( rxMessage->Rx_Header.ID >> 14 ) & 0xF;

	uint8_t tempCommand =
			( ( rxMessage->Data[0] >> 24 ) & 0xF );

	uint8_t addressServo =
			( (rxMessage->Rx_Header.ID >> 7 ) & 0x7F);

	if( tempCommandTechnologicalProtorol == commandAnswerTechnologicalProtorol &&
		tempCommand == commandAnswerParameter &&
		rxMessage->Rx_Header.DLC >= 3  &&
		rxMessage->Rx_Header.DLC <= 7 &&
		addressServo >= 8 && addressServo <= 21)
	{
		uint8_t tempNumberStruct = 0;

		if(	addressServo >= 15 &&
			addressServo <= 21 )
		{
			//number 0 - 6 in struct
			tempNumberStruct = addressServo - 15;
		}
		else
		{
			//number 7 - 13 in struct
			tempNumberStruct = addressServo - 1;
		}

		uint16_t tempParemetr =
				( ( rxMessage->Data[0] >> 8 ) & 0xFFFF );

		//Сейчас объясню что тут происходит:
		//1)формируется uint32_t из d3 | d4 | d5 | d6
		//2)очищается лишняя информация в зависимости от длины посылки,
		//потому-что аппаратно это не происходит
		//3)swapBytes32 для передачи в переменные через функцию answerHandlerCAN
		uint32_t tempData =
				swapBytes32(
						( ( ( rxMessage->Data[0] & 0xFF ) << 24 ) | ( ( rxMessage->Data[1] >> 8 ) & 0xFFFFFF ) )
							& ( 0xFFFFFFFF << ( ( 4 - ( rxMessage->Rx_Header.DLC - 3 ) ) * 8 ) )
							);

		servo->answerHandlerCAN( tempNumberStruct,
								 tempParemetr,
								 tempData );
	}
}

void CanNetworking::canHandler()
{
	CAN_RxMsgTypeDef RxMessage;

	//!Errata!
	for (uint8_t i = CAN_RX_MAX_BUFFER_NUM; MDR_CAN1->TX && i < 32; i++){
		if (getBit(MDR_CAN1->TX, i) && CAN_GetTxITStatus(MDR_CAN1, i))
		{
			CAN_ITClearRxTxPendingBit(MDR_CAN1, i, CAN_STATUS_TX_READY);
			CAN_BufferRelease(MDR_CAN1, i);
			MDR_CAN1->BUF_CON[i] = 0;
		}
	}

	if ( getBit(MDR_CAN1->STATUS, 10) ){
		//over error, reinit can
		initCan1();
	}


	if ( getBit(MDR_CAN2->STATUS, 10) ){
		//over error, reinit can
		initCan2();
	}

	//clear can errors
	MDR_CAN1->STATUS &= ~(0x1FC);
	MDR_CAN2->STATUS &= ~(0x1FC);

	for (uint8_t i = 0; MDR_CAN1->RX != 0 && i < CAN_RX_MAX_BUFFER_NUM; i++)
	{
		if (getBit(MDR_CAN1->RX, i))
		{
			toZeroByteFunction((uint8_t*) &RxMessage, sizeof(CAN_RxMsgTypeDef));
			CAN_GetRawReceivedData(MDR_CAN1, i, &RxMessage);
			canRecieve(i, &RxMessage);
			CAN_ITClearRxTxPendingBit(MDR_CAN1, i, CAN_STATUS_RX_READY);
		}
	}

	for (uint8_t i = 0; MDR_CAN2->RX != 0 && i < CAN_RX_MAX_BUFFER_NUM; i++)
	{
		if (getBit(MDR_CAN2->RX, i))
		{
			toZeroByteFunction((uint8_t*) &RxMessage, sizeof(CAN_RxMsgTypeDef));
			CAN_GetRawReceivedData(MDR_CAN2, i, &RxMessage);
			canRecieve(i, &RxMessage);
			CAN_ITClearRxTxPendingBit(MDR_CAN2, i, CAN_STATUS_RX_READY);
		}
	}

	//!Errata!
	if (MDR_CAN1->RXID & 0x3FFFF){
		MDR_CAN1->RXID = 0;
	}
}

void CanNetworking::initCan1()
{
	MTR_Can_enableClock(CAN1);
	MTR_Can_setSpeed(CAN1, canSpeed1Mbit);
	MTR_Can_setRxPin(CAN1, CAN1_RX);
	MTR_Can_setTxPin(CAN1, CAN1_TX);
	MTR_Can_initGPIO(CAN1);
	MTR_Can_init(CAN1);
	MTR_Can_initReceive(CAN1);

    MDR_CAN1->CONTROL |= 1 << 3; //Send ACK on own packets

	CAN_ITConfig(MDR_CAN1, CAN_IT_ERRINTEN | CAN_IT_ERROVERINTEN , DISABLE);

	for (uint8_t i = 0; i < 7; i++)
	{
		uint32_t ID =
				commandAnswerTechnologicalProtorol  	<< 14 	|
				servo->getServo(i)->online.addressCan	<< 7 	|
				addressSourceMessage		 			<< 0;

		MTR_Can_initFilter(CAN1, i, ID, 0xFFFFF);
	}

	//not read other message
	for (uint8_t i = 7; i < CAN_RX_MAX_BUFFER_NUM; i++)
	{
		MTR_Can_initFilter(CAN1, i, 0xFFFFFFFF, 0x0);
	}

	MTR_Can_start(CAN1);
}

void CanNetworking::initCan2()
{
	MTR_Can_enableClock(CAN2);
	MTR_Can_setSpeed(CAN2, canSpeed1Mbit);
	MTR_Can_setRxPin(CAN2, CAN2_RX);
	MTR_Can_setTxPin(CAN2, CAN2_TX);

	MTR_Can_initGPIO(CAN2);

	MTR_Can_init(CAN2);
	MTR_Can_initReceive(CAN2);

    MDR_CAN2->CONTROL |= 1 << 3; //Send ACK on own packets

	CAN_ITConfig(MDR_CAN2, CAN_IT_ERRINTEN | CAN_IT_ERROVERINTEN , DISABLE);

	for (uint8_t i = 0; i < 7; i++)
	{
		uint32_t ID =
				commandAnswerTechnologicalProtorol  		<< 14 	|
				servo->getServo( i + 7 )->online.addressCan	<< 7 	|
				addressSourceMessage		 				<< 0;

		MTR_Can_initFilter(CAN2, i, ID, 0xFFFFF);
	}

	//not read other message
	for (uint8_t i = 7; i < CAN_RX_MAX_BUFFER_NUM; i++)
	{
		MTR_Can_initFilter(CAN2, i, 0xFFFFFFFF, 0x0);
	}

	MTR_Can_start(CAN2);
}

extern "C" void CAN1_IRQHandler()
{
	__disable_irq();

	for ( uint8_t i = CAN_RX_MAX_BUFFER_NUM; MDR_CAN1->TX && i < 32; i++ )
	{
		if (getBit(MDR_CAN1->TX, i) && CAN_GetTxITStatus(MDR_CAN1, i))
		{
			CAN_ITClearRxTxPendingBit(MDR_CAN1, i, CAN_STATUS_TX_READY);
			CAN_BufferRelease(MDR_CAN1, i);
			MDR_CAN1->BUF_CON[i] = 0;
		}
	}

	__enable_irq();
}

extern "C" void CAN2_IRQHandler()
{
	__disable_irq();

	for ( uint8_t i = CAN_RX_MAX_BUFFER_NUM; MDR_CAN2->TX && i < 32; i++ )
	{
		if (getBit(MDR_CAN2->TX, i) && CAN_GetTxITStatus(MDR_CAN2, i))
		{
			CAN_ITClearRxTxPendingBit(MDR_CAN2, i, CAN_STATUS_TX_READY);
			CAN_BufferRelease(MDR_CAN2, i);
			MDR_CAN2->BUF_CON[i] = 0;
		}
	}

	__enable_irq();
}

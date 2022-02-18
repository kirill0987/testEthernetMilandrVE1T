#include <classes/DataModel/Servo.h>
#include "mtr_service/mtr_service.h"

Servo::Servo()
{
	memset( _servos,  0, sizeof(_servos) );

	//CAN1
	_servos[0].online.addressCan 	= 15;	//pusher_AB13
	_servos[1].online.addressCan 	= 16;	//pusher_AB14
	_servos[2].online.addressCan 	= 17;	//press_AB1
	_servos[3].online.addressCan 	= 18;	//press_AB2
	_servos[4].online.addressCan 	= 19;	//press_AB3
	_servos[5].online.addressCan 	= 20;	//press_AB4
	_servos[6].online.addressCan 	= 21;	//press_AB5

	//CAN2
	_servos[7].online.addressCan 	= 8;	//press_AB6
	_servos[8].online.addressCan 	= 9;	//press_AB7
	_servos[9].online.addressCan 	= 10;	//press_AB8
	_servos[10].online.addressCan 	= 11;	//press_AB9
	_servos[11].online.addressCan 	= 12;	//press_AB10
	_servos[12].online.addressCan 	= 13;	//press_AB11
	_servos[13].online.addressCan 	= 14;	//press_AB12

	//DEBUG
	for(uint8_t i = 0; i < 13; i++)
	{
		_servos[i].setting.speedContour = RPM_LockContour_SCSS;
		_servos[i].setting.rpm = 100;
		_servos[i].setting.current = -0.2;
	}
}

//control
void Servo::controlOnePressServo()
{

}

void Servo::controlOnePusherServo()
{

}

//converts
uint32_t Servo::convertRPM_forWrite(float rpm)
{
	//to IQ12
	return rpm * servo_convertRPM;
}

float Servo::convertRPM_forRead(uint32_t rpm)
{
	return static_cast<float>( static_cast<int32_t>(rpm) / servo_convertRPM );
}

uint32_t Servo::convertCurrent_forWrite(float current)
{
	return static_cast<uint32_t>( ( current * 2500.0 ) / 5.0 );
}

float Servo::convertCurrent_forRead(uint16_t current)
{
	return  ( static_cast<float>( static_cast<int16_t>(current) ) * 5.0 ) / 2500.0;
}

//handler
void Servo::requestAndSetHandlerCAN( uint8_t numberServoInStruct )
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
	*/
}

void Servo::answerHandlerCAN(	uint8_t numberServoInStruct,
								uint16_t paremetr,
								uint32_t data)
{
	switch( paremetr )
	{
		case( speedContourState_vp7 ):
		{
			_servos[numberServoInStruct].feedBack.speedContour =
					static_cast<ServoSpeedContourState_t>(data);
		}
		break;

		case( statePortInputOutput_ip5 ):
		{
			_servos[numberServoInStruct].feedBack.DI = data;
		}
		break;

		case( current_Ct1 ):
		{
			_servos[numberServoInStruct].feedBack.current =
					convertCurrent_forRead( data );
		}
		break;

		case( RPM_Ct12 ):
		{
			_servos[numberServoInStruct].feedBack.rpm =
					convertRPM_forRead( data );
		}
		break;

		case( currentPosition_DD8 ):
		{
			_servos[numberServoInStruct].feedBack.position =
					static_cast<int32_t>( data );
		}
		break;

		case( stateController_DD11 ):
		{
			_servos[numberServoInStruct].feedBack.stateControolerServo = data;
		}
		break;

		default:
		{
			//other parameter or error parameter
		}
		break;
	}
}

//GETTER
DataServo_t* Servo::getServo(uint8_t number)
{
	return &(_servos[number]);
}

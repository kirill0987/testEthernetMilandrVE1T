#include "Addressing/addressing.h"

#include "mtr_libs/mtr_eeprom/mtr_eeprom.h"
#include "mtr_libs/mtr_service/mtr_service.h"
#include <string.h>

BaseAddressing_t baseAddressing;

void Addressing_initData(MTR_Modbus_t* modbus)
{
	baseAddressing.modbusInternalRS = modbus;
	baseAddressing.slaveDevice = 0;
	baseAddressing.addressAssignmentMode = false;
	baseAddressing.addressingState = StateNone_Addressing;

	baseAddressing.addressToAssign = 0;

	baseAddressing.addressWasAssigned = false;
	baseAddressing.addressingReset = false;

	baseAddressing.addressWasAssignedBefore =
			(bool_t)((~MTR_Parameter_read(PARAMETER_PROGRAMM_PROFILE)) & 0x1);

	baseAddressing.stageInternalRS = -1;

	MTR_Planner_addTask( Addressing_modbusPoll, 30, false, NetworkTask );
}

void Addressing_buttonHandler(ButtonEvents_t buttonEvent)
{
	switch (buttonEvent)
	{
		//Entering addressing mode
		case( ButtonPressed_AssignmentModeMaster_Addressing ):
		{
			baseAddressing.addressAssignmentMode = true;
			baseAddressing.addressWasAssigned = false;
			baseAddressing.addressingReset = false;

			baseAddressing.addressToAssign = 0;
			baseAddressing.stageInternalRS = -1;
		}
		break;

		//Leaving addressing mode
		case( ButtonReleased_AssignmentModeMaster_Addressing ):
		{
			baseAddressing.addressingState = StateNone_Addressing;
			MTR_Planner_pause( MTR_Planner_getTask( Addressing_modbusPoll ) );

			//Perform EEPROM operations if needed
			if ( baseAddressing.addressingReset &&
				 baseAddressing.addressWasAssignedBefore)
			{
				MTR_Parameter_write(PARAMETER_PROGRAMM_PROFILE, ~allowedWorkThisModule);
				baseAddressing.addressWasAssignedBefore = false;
			}
			else
			{
				if( baseAddressing.addressWasAssigned &&
					!baseAddressing.addressWasAssignedBefore)
				{
					MTR_Parameter_write(PARAMETER_PROGRAMM_PROFILE, ~notAllowedWorkThisModule);
					baseAddressing.addressWasAssignedBefore = true;
				}
			}
			baseAddressing.addressAssignmentMode = false;
		}
		break;

		//Selecting address when in addressing mode
		case( ButtonPressed_IncrementAddressSlaveForAssignmentModeMaster_Addressing ):
		{
			if (baseAddressing.addressAssignmentMode)
			{
				baseAddressing.addressToAssign++;
				baseAddressing.addressingState = StateAddressSelected_Addressing;

				baseAddressing.slaveDevice =
						MTR_Modbus_master_getDevice(
								baseAddressing.modbusInternalRS,
								baseAddressing.addressToAssign );

				Addressing_resetCounterDevice();

				if( !( MTR_Planner_getTask( Addressing_modbusPoll )->enabled ) )
				{
					MTR_Planner_resume( MTR_Planner_getTask( Addressing_modbusPoll ) );
				}
			}
		}
		break;

		//Checking slave button
		case( ButtonPressed_FeedbackWriteAddressSlave_Addressing ):
		{
			if( baseAddressing.addressAssignmentMode )
			{
				switch (baseAddressing.addressingState)
				{
					//When slave was pressed, but SB1 is not, stop polling (we are not main)
					case( StateNone_Addressing ):
						baseAddressing.addressingReset = true;
					break;

					//Slave was pressed when address is ready - change state
					case( StateAddressSelected_Addressing ):
						baseAddressing.addressingState = StateSlavePressedButton_Addressing;
					break;

					default:
					break;
				}
			}
		}
		break;

		//Checking slave button
		case( ButtonReleased_FeedbackWriteAddressSlave_Addressing ):
		{
			if (baseAddressing.addressingState == StateSlavePressedButton_Addressing)
			{
				baseAddressing.addressingState = StateSlaveReleasedButton_Addressing;
			}
		}
		break;

		default:
		break;
	}
}

void Addressing_modbusPoll()
{
	if ( baseAddressing.modbusInternalRS->status == MODBUS_READY &&
		 baseAddressing.addressingState != StateNone_Addressing )
	{
		uint8_t pollingAddress = slaveDefaultAddress;
		uint16_t addressData = (uint16_t)baseAddressing.addressToAssign;

		if ( baseAddressing.addressingState == StateSlaveReleasedButton_Addressing ||
			 baseAddressing.addressingState == StateAddressAssigned_Addressing)
		{
			pollingAddress = baseAddressing.addressToAssign;
		}

		baseAddressing.stageInternalRS++;

		switch( baseAddressing.stageInternalRS )
		{
			case 0:
			{
				MTR_Modbus_master_setRegister(
						baseAddressing.modbusInternalRS,
						pollingAddress,
						0,
						&addressData);
			}
			break;

			default:
			{
				baseAddressing.stageInternalRS = -1;

				baseAddressing.slaveDevice =
						MTR_Modbus_master_getDevice(
								baseAddressing.modbusInternalRS,
								baseAddressing.addressToAssign );

				if ( baseAddressing.slaveDevice->online &&
					 baseAddressing.addressingState == StateSlaveReleasedButton_Addressing)
				{
					baseAddressing.addressingState = StateAddressAssigned_Addressing;
					baseAddressing.addressWasAssigned = true;
				}
			}
			break;
		}
	}
}

void Addressing_resetCounterDevice()
{
	baseAddressing.slaveDevice->txCounter = 0;
	baseAddressing.slaveDevice->rxCounter = 0;
	baseAddressing.slaveDevice->rxGoodCounter = 0;
	baseAddressing.slaveDevice->noAnswerErrorCounter = 0;
	baseAddressing.slaveDevice->recieveError = 0;
	baseAddressing.slaveDevice->otherError = 0;
	baseAddressing.slaveDevice->statusCounter = 0;
	baseAddressing.slaveDevice->lastRequestHasReply = 0;
	baseAddressing.slaveDevice->online = false;
}

bool_t Addressing_getPreviousAssignment()
{
	return baseAddressing.addressWasAssignedBefore;
}

bool_t Addressing_getAssignmentMode()
{
	return baseAddressing.addressAssignmentMode;
}

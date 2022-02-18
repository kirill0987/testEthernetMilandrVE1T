#include <cstring>

#include "mtr_libs/mtr_modbus/mtr_modbus.h"
#include "mtr_libs/mtr_planner/mtr_planner.h"
#include "mtr_libs/mtr_eeprom/mtr_eeprom.h"
#include "mtr_libs/mtr_button/mtr_button.h"
#include "mtr_libs/mtr_led/mtr_led.h"
#include "mtr_libs/Addressing/addressing.h"
#include "mtr_libs/SlaveModbusDevice/SlaveModbusDevice.h"

#include "inc/inits.h"
#include "inc/tasks.h"
#include "inc/device.h"

#ifdef CONFIG_ETHERNET_TIMER
extern MDR_TIMER_TypeDef * ethTimer;
#endif

extern MTR_Modbus_t* modbusUart1;
extern MTR_Modbus_t* modbusUart2;

extern SlaveModbusDevice<2, 1>* moduleDI1;
extern SlaveModbusDevice<2, 1>* moduleDI2;
extern SlaveModbusDevice<2, 1>* moduleDI3;
extern SlaveModbusDevice<2, 1>* moduleDI4;
extern SlaveModbusDevice<1, 2>* moduleDO1;
extern SlaveModbusDevice<20, 1>* moduleAI1;

//Slave polling
void modbusPollUart1()
{
	static int stageUart1 = -1;

	if (modbusUart1->status == MODBUS_READY)
	{
		stageUart1++;

		switch ( stageUart1 )
		{
			case 0:
				moduleDI1->pullInputRegisters();
			break;

			case 1:
				moduleDI2->pullInputRegisters();
			break;

			case 2:
				moduleDI3->pullInputRegisters();
			break;

			case 3:
				moduleDI4->pullInputRegisters();
			break;

			case 4:
				moduleDO1->pullInputRegisters();
			break;

			case 5:
				moduleDO1->pullHoldingRegisters();
			break;

			case 6:
				moduleAI1->pullInputRegisters();
			break;

			default:
				stageUart1 = -1;
			break;
		}
	}
}

void modbusPollUart2()
{
	static int stageUart2 = -1;

	if (modbusUart2->status == MODBUS_READY)
	{
		stageUart2++;

		switch ( stageUart2 )
		{
			case 0:
				MTR_Modbus_master_getInputRegisters(modbusUart2, 1, 0, 1);
			break;

			default:
				stageUart2 = -1;
			break;
		}
	}
}

void setBlinkWorkLed()
{
	MTR_Led_on( WORK_LED );
	MTR_Planner_reset( MTR_Planner_getTask( resetBlinkWorkLed ) );
	MTR_Planner_resume( MTR_Planner_getTask( resetBlinkWorkLed ) );
}

void noCommandTimeoutInternalRS()
{
	if( Addressing_getPreviousAssignment() && !Addressing_getAssignmentMode() )
	{
		MTR_Planner_pause( MTR_Planner_getTask( resetBlinkWorkLed ) );
		MTR_Led_on( WORK_LED );
	}
}

void resetBlinkWorkLed()
{
	MTR_Led_off( WORK_LED );
	MTR_Planner_pause( MTR_Planner_getTask( resetBlinkWorkLed ) );
}

void switchWarningLed()
{
	MTR_Led_switch( WARNING_LED );
}

extern "C"
{
	void MTR_Button_buttonPressed(uint32_t code)
	{
		if( getBit( code, ASSIGNMENT_MODE_MASTER_BUTTON ) )
		{
			Addressing_buttonHandler( ButtonPressed_AssignmentModeMaster_Addressing );
			MTR_Planner_pause( MTR_Planner_getTask( modbusPollUart1 ) );
			MTR_Planner_pause( MTR_Planner_getTask( switchWarningLed ) );
			MTR_Led_on( WARNING_LED );
			MTR_Led_off( WORK_LED );
		}

		if( getBit( code, INCREMENT_ADDRESS_SLAVE_FOR_ASSIGNMENT_MODE_MASTER_BUTTON ) )
		{
			Addressing_buttonHandler( ButtonPressed_IncrementAddressSlaveForAssignmentModeMaster_Addressing );
			if ( Addressing_getAssignmentMode() )
			{
				MTR_Planner_pause( MTR_Planner_getTask( resetBlinkWorkLed ) );
				MTR_Led_on( WORK_LED );
			}
		}

		if( getBit( code, FEEDBACK_WIRTE_ADDRESS_SLAVE_BUTTON ) )
		{
			Addressing_buttonHandler( ButtonPressed_FeedbackWriteAddressSlave_Addressing );
		}
	}

	void MTR_Button_buttonReleased(uint32_t code)
	{
		if( getBit( code, ASSIGNMENT_MODE_MASTER_BUTTON ) )
		{
			Addressing_buttonHandler( ButtonReleased_AssignmentModeMaster_Addressing );

			MTR_Led_off( WORK_LED );
			MTR_Led_off( WARNING_LED );
			MTR_Planner_reset( MTR_Planner_getTask( noCommandTimeoutInternalRS ) );

			if( Addressing_getPreviousAssignment() )
			{
				MTR_Planner_resume( MTR_Planner_getTask( modbusPollUart1 ) );
				//192.168.1.10
				MTR_Tcp_setAddress(0xC0A8010A);
			}
			else
			{
				MTR_Planner_reset( MTR_Planner_getTask( switchWarningLed ) ) ;
				MTR_Planner_resume( MTR_Planner_getTask( switchWarningLed ) ) ;
				//192.168.1.11
				MTR_Tcp_setAddress(0xC0A8010B);
			}
		}

		if( getBit( code, FEEDBACK_WIRTE_ADDRESS_SLAVE_BUTTON ) )
		{
			Addressing_buttonHandler( ButtonReleased_FeedbackWriteAddressSlave_Addressing );
		}
	}

	void MTR_Modbus_master_callbackAfterProccess(MTR_Modbus_t* modbus)
	{
		if( modbus->connection == Uart1Connection )
		{
			setBlinkWorkLed();
			MTR_Planner_reset( MTR_Planner_getTask( noCommandTimeoutInternalRS ) );
		}
	}

#ifdef CONFIG_ETHERNET_TIMER
	void CONFIG_ETHERNET_TIMER_IRQ()
	{
		MTR_Ethernet_perform();
		MTR_Tcp_doNetworkStuff(MDR_ETHERNET1);
		ethTimer->CNT = 0;
		ethTimer->STATUS &= ~TIMER_STATUS_CNT_ARR;
	}
#endif
}

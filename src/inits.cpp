#include <malloc.h>

#include "mtr_libs/Addressing/addressing.h"
#include "mtr_libs/mtr_modbus/mtr_modbus.h"
#include "mtr_libs/mtr_planner/mtr_planner.h"
#include "mtr_libs/mtr_modbus_base/mtr_modbus_base.h"
#include "mtr_libs/mtr_modbus_slave/mtr_modbus_slave.h"
#include "mtr_libs/mtr_ethernet/mtr_ethernet.h"
#include "mtr_libs/mtr_tcpip/mtr_tcpip.h"
#include "mtr_libs/mtr_led/mtr_led.h"
#include "mtr_libs/mtr_button/mtr_button.h"
#include "mtr_libs/SlaveModbusDevice/SlaveModbusDevice.h"

#include "classes/NetworkCAN/CanNetworking.h"
#include "classes/DataManager.h"

#include "classes/DataModel/Servo.h"

#include "inc/tasks.h"
#include "inc/device.h"

uint16_t inputRegister[INPUT_REGISTERS_AMOUNT] = {0};
uint16_t holdingRegister [HOLDING_REGISTERS_AMOUNT] = {0};

MTR_Modbus_t* modbusUart1 		= nullptr;
MTR_Modbus_t* modbusUart2 		= nullptr;
MTR_Modbus_t* modbusEthernet 	= nullptr;
CanNetworking* canNetworking	= nullptr;
DataManager* dataManager		= nullptr;

SlaveModbusDevice<2, 1>* moduleDI1 = nullptr;
SlaveModbusDevice<2, 1>* moduleDI2 = nullptr;
SlaveModbusDevice<2, 1>* moduleDI3 = nullptr;
SlaveModbusDevice<2, 1>* moduleDI4 = nullptr;
SlaveModbusDevice<1, 2>* moduleDO1 = nullptr;
SlaveModbusDevice<20, 1>* moduleAI1 = nullptr;

Servo* servo = nullptr;

#ifdef CONFIG_ETHERNET_TIMER
MDR_TIMER_TypeDef* ethTimer;
#endif

void initBeforeData()
{
	servo = new Servo();

	dataManager =
		new DataManager();

	canNetworking =
		new CanNetworking( 0x00 , dataManager );
}

void initGPIO()
{
	MTR_Led_init(	WORK_LED,
					CONTACT_WORK_LED );

	MTR_Led_off( WORK_LED );

	MTR_Led_init(	WARNING_LED,
					CONTACT_WARNING_LED );

	MTR_Led_off( WARNING_LED );

	MTR_Button_init(	INCREMENT_ADDRESS_SLAVE_FOR_ASSIGNMENT_MODE_MASTER_BUTTON,
						CONTACT_INCREMENT_ADDRESS_ASSIGNMENT_MODE_MASTER_BUTTON,
						ReverseButton);

	MTR_Button_init(	ASSIGNMENT_MODE_MASTER_BUTTON,
						CONTACT_ASSIGNMENT_MODE_MASTER_BUTTON,
						NormalButton);

	MTR_Button_init(	FEEDBACK_WIRTE_ADDRESS_SLAVE_BUTTON,
						CONTACT_FEEDBACK_WIRTE_ADDRESS_SLAVE_BUTTON,
						NormalButton);

	MTR_Port_initDigital(POWER_EXTERNAL_RS_ON_INTERFACE_EXPANSION_BOARD, OUT);
	MTR_Port_setPin(POWER_EXTERNAL_RS_ON_INTERFACE_EXPANSION_BOARD);

	MTR_Port_initDigital(CAN1_ENABLE_TX_RX, OUT);
	MTR_Port_setPin(CAN1_ENABLE_TX_RX);
}

void initNetwork()
{
	//--RS--
	MTR_Uart_setEnPin(INTERNAL_RS, ENABLE_INTERNAL_RS);
	MTR_Uart_setTxPin(INTERNAL_RS, TX_INTERNAL_RS);
	MTR_Uart_setRxPin(INTERNAL_RS, RX_INTERNAL_RS);
	MTR_Uart_setType(INTERNAL_RS, RS485);
	MTR_Uart_setSpeed(INTERNAL_RS, 57600);

	modbusUart1 = MTR_Modbus_master_init(INTERNAL_RS);

	MTR_Uart_setEnPin(EXTERNAL_RS, ENABLE_EXTERNAL_RSON_INTERFACE_EXPANSION_BOARD);
	MTR_Uart_setTxPin(EXTERNAL_RS, TX_EXTERNAL_RS);
	MTR_Uart_setRxPin(EXTERNAL_RS, RX_EXTERNAL_RS);
	MTR_Uart_setType(EXTERNAL_RS, RS485);
	MTR_Uart_setSpeed(EXTERNAL_RS, 57600);

	modbusUart2 = MTR_Modbus_master_init(EXTERNAL_RS);

	//--CAN--
	canNetworking->initCan1();

	//Internal RS addressing initialization
	Addressing_initData( modbusUart1 );

	//--Ethernet--
	//Use this if there are two masters
	if( Addressing_getPreviousAssignment() )
	{
		//192.168.1.10
		MTR_Tcp_setAddress(0xC0A8010A);
	}
	else
	{
		//192.168.1.11
		MTR_Tcp_setAddress(0xC0A8010B);
	}

	//192.168.1.1
	MTR_Tcp_setGateway(0xC0A80101);

	//255.255.0.0
	MTR_Tcp_setMask(0xFFFF0000);

	modbusEthernet = MTR_Modbus_slave_init(Ethernet, 1);

	for ( uint8_t i = 0; i < INPUT_REGISTERS_AMOUNT; i++ )
	{
		MTR_Modbus_slave_addInputRegister(modbusEthernet, i, &inputRegister[i]);
	}

	for ( uint8_t i = 0; i < HOLDING_REGISTERS_AMOUNT; i++ )
	{
		MTR_Modbus_slave_addHoldingRegister(modbusEthernet, i, &holdingRegister[i]);
	}

#ifdef CONFIG_ETHERNET_TIMER
	MTR_Timer_init(CONFIG_ETHERNET_TIMER, 0, 1, (uint16_t) (CONFIG_CLOCK_TIMER_HZ / 1000));
	MTR_Timer_initIT(CONFIG_ETHERNET_TIMER, TIMER_STATUS_CNT_ARR, enable);
	MTR_Timer_initNvic(CONFIG_ETHERNET_TIMER);
	MTR_Timer_setPriority(CONFIG_ETHERNET_TIMER, 0);
	NVIC_SetPriority(MTR_Timer_getIrq(CONFIG_ETHERNET_TIMER), 0);
	ethTimer = MTR_Timer_getOriginStruct(CONFIG_ETHERNET_TIMER);
	MTR_Timer_start(CONFIG_ETHERNET_TIMER);
#endif

	NVIC_SetPriority(UART1_IRQn, 2);
	NVIC_SetPriority(UART2_IRQn, 2);
	NVIC_SetPriority(MTR_Timer_getIrq(CONFIG_PLANNER_TIMER), 3);
	NVIC_SetPriority(MTR_Timer_getIrq(CONFIG_UART_TIMER), 4);
}

void initAfterData()
{
	moduleDI1 = new SlaveModbusDevice<2, 1>(2, modbusUart1);

	moduleDI2 = new SlaveModbusDevice<2, 1>(3, modbusUart1);

	moduleDI3 = new SlaveModbusDevice<2, 1>(4, modbusUart1);

	moduleDI4 = new SlaveModbusDevice<2, 1>(5, modbusUart1);

	moduleDO1 = new SlaveModbusDevice<1, 2>(6, modbusUart1);

	moduleAI1 = new SlaveModbusDevice<20, 1>(7, modbusUart1);
}

void initPlanner()
{
	//--Button--
	MTR_Planner_addTask( MTR_Button_update, 10, true, UpdateTask );

	//--Nwtwork RS--
	MTR_Planner_addTask( modbusPollUart1, 30, Addressing_getPreviousAssignment(), NetworkTask );
	MTR_Planner_addTask( modbusPollUart2, 30, true, NetworkTask );
	MTR_Planner_addTask( noCommandTimeoutInternalRS, 3000, true, NetworkTask);

	//--CAN Receiver--
	MTR_Planner_addTask([](){canNetworking->canHandler();}, 1, true, NetworkTask);

	//--CAN Transmitter--
	MTR_Planner_addTask([](){canNetworking->initFrameServoCan1();}, 5, true, NetworkTask);

	//--LEDs--
	MTR_Planner_addTask( resetBlinkWorkLed, 10, false, UpdateTask );
	MTR_Planner_addTask( switchWarningLed, 100, !Addressing_getPreviousAssignment(), UpdateTask );

#ifndef CONFIG_ETHERNET_TIMER
	MTR_Planner_addTask(
			[](){
					MTR_Ethernet_perform();
					MTR_Tcp_doNetworkStuff(MDR_ETHERNET1);
				}, 1, true, UpdateTask);
#endif

	MTR_Planner_reset( MTR_Planner_getTask( noCommandTimeoutInternalRS ) );
	MTR_Planner_reset( MTR_Planner_getTask( resetBlinkWorkLed ) );
    MTR_Planner_init();
}

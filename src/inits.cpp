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
#include "cmsis_lib/inc/MDR32F9Qx_eth.h"
#include "MDR32F9Qx_rst_clk.h"

#include "inc/tasks.h"
#include "inc/device.h"


#define MAC_5				0xC8
#define MAC_4				0x60
#define MAC_3				0x00
#define MAC_2				0x67
#define MAC_1				0xB1
#define MAC_0				0x6D

uint32_t InputFrame[1514/4];

void initData()
{

}

void initGPIO()
{

}

void initNetwork()
{
	static ETH_InitTypeDef  ETH_InitStruct;

	//Сброс clock registers по умолчанию
	ETH_ClockDeInit();

	RST_CLK_PCLKcmd(RST_CLK_PCLK_DMA, ENABLE);//?? Errata

	// Enable HSE2 oscillator
	RST_CLK_HSE2config(RST_CLK_HSE2_ON);
	if(RST_CLK_HSE2status() == ERROR)
	{
		while(1);
		//Если ошибка жри говно в бесконечном цикле,
		//ибо ошибок не должно быть
	}

	//Config PHY clock
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);

	//Init the BRG ETHERNET
	ETH_BRGInit(ETH_HCLKdiv1);

	//Enable the ETHERNET clock
	ETH_ClockCMD(ETH_CLK1, ENABLE);

	ETH_DeInit(MDR_ETHERNET1);

	//Заполнение структуры значениям по умолчанию
	ETH_StructInit(&ETH_InitStruct);

	//Set the speed of the chennel
	ETH_InitStruct.ETH_PHY_Mode = ETH_PHY_MODE_AutoNegotiation;
	ETH_InitStruct.ETH_Transmitter_RST = SET;
	ETH_InitStruct.ETH_Receiver_RST = SET;

	//Set the buffer mode
	ETH_InitStruct.ETH_Buffer_Mode = ETH_BUFFER_MODE_LINEAR;

	ETH_InitStruct.ETH_Source_Addr_HASH_Filter = DISABLE;

	//Set the MAC address.
	ETH_InitStruct.ETH_MAC_Address[2] = (MAC_0<<8)|MAC_1;
	ETH_InitStruct.ETH_MAC_Address[1] = (MAC_2<<8)|MAC_3;
	ETH_InitStruct.ETH_MAC_Address[0] = (MAC_4<<8)|MAC_5;

	ETH_InitStruct.ETH_Delimiter = 0x1000;

	//Перенос абстракции в регистры
	ETH_Init(MDR_ETHERNET1, &ETH_InitStruct);

	//Enable PHY module
	ETH_PHYCmd(MDR_ETHERNET1, ENABLE);

	//Запуск ehetnet
	ETH_Start(MDR_ETHERNET1);

}

void initPlanner()
{
    MTR_Planner_init();
}

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

extern uint32_t InputFrame[1514/4];

void ETH_InputPachetHandler(MDR_ETHERNET_TypeDef * ETHERNETx)
{
	/*
	uint16_t status_reg;
	ETH_StatusPacketReceptionTypeDef ETH_StatusPacketReceptionStruct;

	//Check that the packet is received
	status_reg = ETH_GetMACITStatusRegister(ETHERNETx);

	if(ETHERNETx->ETH_R_Head != ETHERNETx->ETH_R_Tail){
	//if(status_reg & ETH_MAC_IT_RF_OK ){
		ETH_StatusPacketReceptionStruct.Status = ETH_ReceivedFrame(ETHERNETx, InputFrame);

		if(ETH_StatusPacketReceptionStruct.Fields.UCA)
			 ProcessEthIAFrame(InputFrame, ETH_StatusPacketReceptionStruct.Fields.Length);
		if(ETH_StatusPacketReceptionStruct.Fields.BCA)
			 ProcessEthBroadcastFrame(InputFrame, ETH_StatusPacketReceptionStruct.Fields.Length);
	}
	*/
}

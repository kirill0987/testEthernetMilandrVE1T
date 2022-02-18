#include "mtr_ethernet/mtr_ethernet.h"

#include "MDR32F9Qx_rst_clk.h"

static Ethernet_SettingsStruct_t ethernetSettingsMainStructs[MTR_ETHERNET_AMOUNT] =
{
	{
		MDR_ETHERNET1,
		{
			//default mac and reverse mac
			{0x6D, 0xB1, 0x67, 0x00, 0x60, 0xC8},
			{0xC8, 0x60, 0x00, 0x67, 0xB1, 0x6D}
		}
	}
};

Ethernet_DataBuffer incomeBuffers[MTR_ETHERNET_BUFFER_SIZE] __attribute__((section(".ramfunc"))) __attribute__ ((aligned (4)));
Ethernet_DataBuffer dummyBuffer __attribute__((section(".ramfunc"))) __attribute__ ((aligned (4)));

Ethernet_DataBuffer* MTR_Ethernet_freeBuffer(){
	for (int i = 0; i < MTR_ETHERNET_BUFFER_SIZE; i++){
		if (incomeBuffers[i].used == false){
			incomeBuffers[i].used = true;
			return incomeBuffers + i;
		}
	}
	//all buffer is used
	return 0;
}

void MTR_Ethernet_clockInit()
{
	ETH_ClockDeInit();
	RST_CLK_PCLKcmd(RST_CLK_PCLK_DMA, ENABLE);
	RST_CLK_HSE2config(RST_CLK_HSE2_ON);			// Enable HSE2 oscillator
	if(RST_CLK_HSE2status() == ERROR)
		while(1);

	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);
	ETH_BRGInit(ETH_HCLKdiv1);
	ETH_ClockCMD(ETH_CLK1, ENABLE);
}

void MTR_Ethernet_setMac(AvaliableConnectionType_t ethernet,
						 Ethernet_MacAddress_t mac){
	Ethernet_SettingsStruct_t* ethernetSettings = &ethernetSettingsMainStructs [ethernet - Ethernet];
	ethernetSettings->mac = mac;
}

Ethernet_MacAddress_t const * MTR_Ethernet_mac(AvaliableConnectionType_t ethernet)
{
	Ethernet_SettingsStruct_t* ethernetSettings = &ethernetSettingsMainStructs [ethernet - Ethernet];
	return &ethernetSettings->mac;
}

void MTR_Ethernet_init (AvaliableConnectionType_t ethernet)
{
	Ethernet_SettingsStruct_t* ethernetSettings = &ethernetSettingsMainStructs [ethernet - Ethernet];
	ETH_InitTypeDef  ETH_InitStruct;

	for (int i = 0; i < MTR_ETHERNET_BUFFER_SIZE; i++){
		incomeBuffers[i].used = false;
	}

	MTR_Ethernet_clockInit();

	ETH_DeInit(ethernetSettings->origin);
	ETH_StructInit((ETH_InitTypeDef * ) &ETH_InitStruct);

	ETH_InitStruct.ETH_PHY_Mode = ETH_PHY_MODE_AutoNegotiation;
	ETH_InitStruct.ETH_Buffer_Mode = ETH_BUFFER_MODE_AUTOMATIC_CHANGE_POINTERS;
	ETH_InitStruct.ETH_Transmitter_RST = SET;
	ETH_InitStruct.ETH_Receiver_RST = SET;
	ETH_InitStruct.ETH_Source_Addr_HASH_Filter = DISABLE;

	memcpy(ETH_InitStruct.ETH_MAC_Address, &ethernetSettings->mac.rAddress, MAC_SIZE);

	for (int i = 0; i < MAC_SIZE; i++){
		ethernetSettings->mac.rAddress[i] = ethernetSettings->mac.address[MAC_SIZE - 1 - i];
	}

	ETH_InitStruct.ETH_Delimiter = 0x1000;

	ETH_Init(ethernetSettings->origin, (ETH_InitTypeDef *) &ETH_InitStruct);

	ETH_PHYCmd(ethernetSettings->origin, ENABLE);
}

void MTR_Ethernet_start (AvaliableConnectionType_t ethernet)
{
	Ethernet_SettingsStruct_t* ethernetSettings = &ethernetSettingsMainStructs [ethernet - Ethernet];
	ETH_Start(ethernetSettings->origin);
}

void MTR_Ethernet_deInit (AvaliableConnectionType_t ethernet)
{
	Ethernet_SettingsStruct_t* ethernetSettings = &ethernetSettingsMainStructs [ethernet - Ethernet];

	ETH_ClockCMD(ETH_CLK1, DISABLE);
	ETH_ClockDeInit();
	ETH_DeInit(ethernetSettings->origin);
}

void MTR_Ethernet_perform(){

	uint16_t status_reg;
	ETH_StatusPacketReceptionTypeDef ETH_StatusPacketReceptionStruct;

	/* Check that the packet is received */
	status_reg = ETH_GetMACITStatusRegister(MDR_ETHERNET1);

// Not need checks any flag, info from datasheet
//	if(!(status_reg & ETH_MAC_IT_RF_OK)){
//		return;
//	}

	if(MDR_ETHERNET1->ETH_R_Head != MDR_ETHERNET1->ETH_R_Tail){

		Ethernet_DataBuffer* buffer = MTR_Ethernet_freeBuffer();

		if (buffer == 0){
			buffer = &dummyBuffer;
		}

		//can process
		ETH_StatusPacketReceptionStruct.Status = ETH_ReceivedFrame(MDR_ETHERNET1, buffer);
		buffer->bufferSize = ETH_StatusPacketReceptionStruct.Fields.Length;

		if (buffer != &dummyBuffer){
			if(ETH_StatusPacketReceptionStruct.Fields.BCA){
				 ProcessEthBroadcastFrame(buffer);
			}
			else if(ETH_StatusPacketReceptionStruct.Fields.UCA) {
				ProcessEthIAFrame(buffer);
			}
		}
	}
}

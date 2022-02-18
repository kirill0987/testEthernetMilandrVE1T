#include "mtr_can/mtr_can.h"
#include "mtr_eeprom/mtr_eeprom.h"
#include "MDR32F9Qx_rst_clk.h"

MTR_Can_t CAN[2]=
{
    {
    EXTENDED, MDR_CAN1, CAN1_IRQn,
    {PORTA, PIN6, DIGITAL, ALTER, OUT, MAX, disable, enable},
    {PORTA, PIN7, DIGITAL, ALTER, IN, MAX, disable, enable},
	{PORTA, PIN0, DIGITAL, PORT, OUT, MAX, disable, disable},
	canSpeed500Kbit, RST_CLK_PCLK_CAN1, CAN_HCLKdiv2, false, false},
    {
    EXTENDED, MDR_CAN2, CAN2_IRQn,
    {PORTE, PIN7, DIGITAL, ALTER, OUT, MAX, disable, enable},
    {PORTE, PIN6, DIGITAL, ALTER, IN, MAX, disable, enable},
	{PORTA, PIN0, DIGITAL, PORT, OUT, MAX, disable, disable},
	canSpeed500Kbit, RST_CLK_PCLK_CAN2, CAN_HCLKdiv2, false, false
    }
};

void MTR_Can_setSpeed(MTR_Can_Name_t canName, MTR_Can_Speed_t canSpeed)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
    can->speed = canSpeed;
}

void MTR_Can_setType(MTR_Can_Name_t canName, MTR_Can_Type_t type)
{
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	can->type = type;
}

void MTR_Can_setTxPin(MTR_Can_Name_t canName, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

    can->txPin.func = func;
    can->txPin.portPin = pin;
    can->txPin.portName = port;
    can->txPin.pullDown = false;
    can->txPin.pullUp = false;
    can->txPin.direction = OUT;
    can->txPin.mode = DIGITAL;
    can->txPin.speed = MAX;
}

void MTR_Can_setRxPin(MTR_Can_Name_t canName, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func)
{
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);

   	can->rxPin.func = func;
    can->rxPin.portPin = pin;
    can->rxPin.portName = port;
    can->rxPin.pullDown = false;
    can->rxPin.pullUp = false;
    can->rxPin.direction = IN;
    can->rxPin.mode = DIGITAL;
    can->rxPin.speed = MAX;
}

void MTR_Can_setPowerPin(MTR_Can_Name_t canName, MTR_Port_Name_t port, MTR_Port_Pin_t pin)
{
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);

   	can->powerPin.func = PORT;
    can->powerPin.portPin = pin;
    can->powerPin.portName = port;
    can->powerPin.pullDown = false;
    can->powerPin.pullUp = false;
    can->powerPin.direction = OUT;
    can->powerPin.mode = DIGITAL;
    can->powerPin.speed = MAX;
    can->needPowerPin = true;
}

void MTR_Can_enableClock(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
    RST_CLK_PCLKcmd(can->pclkMask, ENABLE);
}

void MTR_Can_deInit(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	CAN_DeInit(can->originName);
}

CAN_InitTypeDef canInitStuct = {
		DISABLE,
		DISABLE,
		DISABLE, // SELF TEST MODE
		DISABLE, //ENABLE READONLY MODE
		CAN_PSEG_Mul_2TQ, //PSEG
		CAN_SEG1_Mul_2TQ, //SEG1
		CAN_SEG2_Mul_3TQ, //SEG2
		CAN_SJW_Mul_2TQ,
		CAN_SB_1_SAMPLE,
		1,
		255,
};

void MTR_Can_init(MTR_Can_Name_t canName)
{
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	CAN_BRGInit(can->originName, can->hclkDiv);
	CAN_DeInit(can->originName);

	if (can->needPowerPin)
	{
		MTR_Port_initWithStruct(can->powerPin);
		MTR_Port_setPin(can->powerPin.portName, can->powerPin.portPin);
	}

	uint32_t prescaller;
	if (CONFIG_CURRENT_CLOCK_HZ == 128000000)
	{
	    switch (can->speed)
	    {
	        case canSpeed1Mbit: prescaller = 7;
	            break;
	        case canSpeed800Kbit: prescaller = 9;
	            break;
	        case canSpeed500Kbit: prescaller = 15;
	            break;
	        case canSpeed250Kbit: prescaller = 31;
	            break;
	        case canSpeed125Kbit: prescaller = 63;
	            break;
	        case canSpeed100Kbit: prescaller = 79;
	            break;
	        case canSpeed50Kbit: prescaller = 159;
	            break;
	        case canSpeed20Kbit: prescaller = 399;
	            break;
	        case canSpeed10Kbit: prescaller = 799;
	            break;
	    }
	}
	else if (CONFIG_CURRENT_CLOCK_HZ == 64000000) {
	    switch (can->speed)
	    {
	        case canSpeed1Mbit: prescaller = 3;
	            break;
	        case canSpeed800Kbit: prescaller = 4;
	            break;
	        case canSpeed500Kbit: prescaller = 7;
	            break;
	        case canSpeed250Kbit: prescaller = 15;
	            break;
	        case canSpeed125Kbit: prescaller = 31;
	            break;
	        case canSpeed100Kbit: prescaller = 39;
	            break;
	        case canSpeed50Kbit: prescaller = 79;
	            break;
	        case canSpeed20Kbit: prescaller = 199;
	            break;
	        case canSpeed10Kbit: prescaller = 399;
	            break;
	    }
	}

	canInitStuct.CAN_BRP = prescaller;
	canInitStuct.CAN_SAP = can->enableSelfConfirmation ? ENABLE : DISABLE;

	CAN_Init(can->originName, &canInitStuct);

	uint32_t rxChannelBiteNum = 0;

	for (uint8_t i  = 0; i < CAN_RX_MAX_BUFFER_NUM; i++){
		rxChannelBiteNum |= (1 << i);
	}

	CAN_TxITConfig(can->originName, (uint32_t) (~ (uint32_t)rxChannelBiteNum), ENABLE);

	NVIC_EnableIRQ(can->irq);
	NVIC_SetPriority(can->irq, CONFIG_CAN_NVIC_PRIORITY);
	CAN_ITConfig(can->originName, CAN_IT_GLBINTEN | CAN_IT_TXINTEN | CAN_IT_ERRINTEN |CAN_IT_ERROVERINTEN , ENABLE);
}

void MTR_Can_sendStandartMessage(MTR_Can_Name_t canName, uint16_t id, uint8_t length, uint8_t* data)
{
	CAN_TxMsgTypeDef txMessage;
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	txMessage.IDE = CAN_ID_STD;
	txMessage.DLC = length;
	txMessage.PRIOR_0 = DISABLE;
	txMessage.ID  = (0x7FF & id)<< 18;
	txMessage.Data[0] = get32From8Bits(&data[0]);
	txMessage.Data[1] = get32From8Bits(&data[4]);

	uint32_t bufferNum = MTR_Can_getEmptyTransferBuffer(canName);
	CAN_Transmit(can->originName, bufferNum, &txMessage);
}

void MTR_Can_sendExtendedMessage(MTR_Can_Name_t canName, uint32_t id, uint8_t length, uint8_t* data)
{
	CAN_TxMsgTypeDef txMessage;
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	txMessage.IDE = can->type;
	txMessage.DLC = length;
	txMessage.PRIOR_0 = DISABLE;
	txMessage.ID  = 0x1FFFFFFF & id;
	txMessage.Data[0] = get32From8Bits(&data[0]);
	txMessage.Data[1] = get32From8Bits(&data[4]);

	uint32_t bufferNum = MTR_Can_getEmptyTransferBuffer(canName);
	CAN_Transmit(can->originName, bufferNum, &txMessage);
}

void MTR_Can_sendExtendedMessageDirect(MTR_Can_Name_t canName, uint32_t id, uint8_t length, uint8_t* data)
{
	CAN_TxMsgTypeDef txMessage;
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	txMessage.IDE = can->type;
	txMessage.DLC = length;
	txMessage.PRIOR_0 = DISABLE;
	txMessage.ID  = 0x1FFFFFFF & id;
	txMessage.Data[0] = *((uint32_t*)data);
	txMessage.Data[1] = *((uint32_t*)data + 1);

	uint32_t bufferNum = MTR_Can_getEmptyTransferBuffer(canName);
	CAN_Transmit(can->originName, bufferNum, &txMessage);
}

void MTR_Can_sendRtrMessage(MTR_Can_Name_t canName, MTR_Can_Type_t type, uint32_t id, uint8_t length)
{
	CAN_RTRMessageTypeDef rtrMessage;
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	rtrMessage.ID = type ? id  : (0x7FF & id) << 18;
	rtrMessage.IDE = type;
	rtrMessage.PRIOR_0 = ENABLE;
	rtrMessage.DLC = length;

	uint32_t bufferNum = MTR_Can_getEmptyTransferBuffer(canName);

	CAN_SendRTR(can->originName, bufferNum, &rtrMessage);
}

void MTR_Can_start(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	CAN_Cmd(can->originName, ENABLE);
}

void MTR_Can_stop(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	CAN_Cmd(can->originName, DISABLE);
}

void MTR_Can_initRxIT(MTR_Can_Name_t canName, bool_t enableRxIT)
{
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);;
	CAN_ITConfig(can->originName, CAN_IT_RXINTEN , enableRxIT);
}

void MTR_Can_ITclearRxFlag(MTR_Can_Name_t canName, uint8_t rxBufferNumber)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	CAN_ITClearRxTxPendingBit(can->originName, rxBufferNumber, CAN_STATUS_RX_READY);
}

void MTR_Can_ITclearTxFlag(MTR_Can_Name_t canName, uint8_t txBufferNumber)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	CAN_ITClearRxTxPendingBit(can->originName, txBufferNumber, CAN_STATUS_TX_READY);
}

void MTR_Can_initGPIO(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
    MTR_Port_initWithStruct(can->txPin);
    MTR_Port_initWithStruct(can->rxPin);
}

void MTR_Can_initReceive(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

    for (uint16_t i = 0; i < CAN_RX_MAX_BUFFER_NUM; i++) {
		CAN_Receive(can->originName, i, ENABLE);
		CAN_RxITConfig(can->originName, 1 << i, ENABLE);
	}
}

void MTR_Can_initFilter(MTR_Can_Name_t canName, uint32_t rxBufferNum, uint32_t filter, uint32_t mask)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

    CAN_FilterInitTypeDef canFilterStruct;

	canFilterStruct.Filter_ID = filter;
	canFilterStruct.Mask_ID =  mask;

	CAN_FilterInit(can->originName, rxBufferNum, &canFilterStruct);
}

MTR_Can_Status_t* Mtr_Can_getStatus(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

    static MTR_Can_Status_t statusOut;

    uint32_t statusRaw = CAN_GetStatus(can->originName);

    statusOut.txErrorCounter = (statusRaw >> CAN_STATUS_SHIFT_TX_ERROR_COUNTER) & CAN_STATUS_MASK_TX_ERROR_COUNTER;
    statusOut.rxErrorCounter = (statusRaw >> CAN_STATUS_SHIFT_RX_ERROR_COUNTER) & CAN_STATUS_MASK_RX_ERROR_COUNTER;
    statusOut.txErrorOver = (statusRaw >> CAN_STATUS_SHIFT_TX_ERROR_OVER) & CAN_STATUS_MASK_TX_ERROR_OVER;
    statusOut.rxErrorOver = (statusRaw >> CAN_STATUS_SHIFT_RX_ERROR_OVER) & CAN_STATUS_MASK_RX_ERROR_OVER;
    statusOut.errorStatus = (statusRaw >> CAN_STATUS_SHIFT_ERROR_STATUS) & CAN_STATUS_MASK_ERROR_STATUS;
    statusOut.idLower = (statusRaw >> CAN_STATUS_SHIFT_ID_LOWER) & CAN_STATUS_MASK_ID_LOWER;
    statusOut.ackError = (statusRaw >> CAN_STATUS_SHIFT_ACK_ERROR) & CAN_STATUS_MASK_ACK_ERROR;
    statusOut.frameError = (statusRaw >> CAN_STATUS_SHIFT_FRAME_ERROR) & CAN_STATUS_MASK_FRAME_ERROR;
    statusOut.crcError = (statusRaw >> CAN_STATUS_SHIFT_CRC_ERROR) & CAN_STATUS_MASK_CRC_ERROR;
    statusOut.bitStuffError = (statusRaw >> CAN_STATUS_SHIFT_BIT_STUFF_ERROR) & CAN_STATUS_MASK_BIT_STUFF_ERROR;
    statusOut.bitError = (statusRaw >> CAN_STATUS_SHIFT_BIT_ERROR) & CAN_STATUS_MASK_BIT_ERROR;
    statusOut.overError = (statusRaw >> CAN_STATUS_SHIFT_OVER_ERROR) & CAN_STATUS_MASK_OVER_ERROR;
    statusOut.txReady = (statusRaw >> CAN_STATUS_SHIFT_TX_READY) & CAN_STATUS_MASK_TX_READY;
    statusOut.rxReady = (statusRaw >> CAN_STATUS_SHIFT_RX_READY) & CAN_STATUS_MASK_RX_READY;

    return &statusOut;
}

MDR_CAN_TypeDef* Mtr_Can_getOrigin(MTR_Can_Name_t canName)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	return can->originName;
}

uint32_t MTR_Can_getEmptyTransferBuffer(MTR_Can_Name_t canName)
{
	uint32_t buffer_number;
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	for (buffer_number = 0; (buffer_number < CAN_BUFFER_NUMBER) &&
        (can->originName->BUF_CON[buffer_number] != 0);
        buffer_number++)
	{}

	return buffer_number;
}

void MTR_Can_enableAutoConfirmation(MTR_Can_Name_t canName)
{
	MTR_Can_t* can = MTR_Can_getMainStruct(canName);
	can->enableSelfConfirmation = true;
}

void MTR_Can_checkMessageForMagicPackage(MTR_Can_Name_t canName, CAN_RxMsgTypeDef* messageToCheck)
{
    MTR_Can_t* can = MTR_Can_getMainStruct(canName);

	if (messageToCheck->Rx_Header.ID == (getDeviceId() & 0x1FFFFFFF)){

		__disable_irq();
		//__disable_fault_irq();

		MTR_Parameter_write(PARAMETER_BOOTSELECT, canName);
		MTR_Parameter_write(PARAMETER_CONNECTION_RXPINPORT, MTR_Port_getUint32FromPin(&can->rxPin));
		MTR_Parameter_write(PARAMETER_CONNECTION_TXPINPORT, MTR_Port_getUint32FromPin(&can->txPin));

		if (can->needPowerPin){
			MTR_Parameter_write(PARAMETER_CONNECTION_EXTRAPINPORT, MTR_Port_getUint32FromPin(&can->powerPin));
		}

		MTR_Parameter_write(PARAMETER_CONNECTION_SPEED_AND_PARITY, (uint32_t) (can->speed | (can->needPowerPin << 29)));

		NVIC_SystemReset();
	}
}

MTR_Can_t* MTR_Can_getMainStruct(MTR_Can_Name_t canName)
{
	if (canName == CAN2){
		return (CAN + 1);
	}
	else {
		return CAN;
	}
}

#include "mtr_usb_cdc/mtr_usb_cdc.h"
#include "mtr_eeprom/mtr_eeprom.h"
#include "mtr_service/mtr_service.h"
#include "MDR32F9Qx_usb_handlers.h"
#include "MDR32F9Qx_rst_clk.h"

volatile uint32_t PendingDataLength = 0;

//from header file
USB_Clock_TypeDef USB_Clock_InitStruct;
USB_DeviceBUSParam_TypeDef USB_DeviceBUSParam;
uint8_t Buffer[BUFFER_LENGTH];
USB_CDC_LineCoding_TypeDef LineCoding;
uint32_t SPIndex;
uint32_t ReceivedByteCount, SentByteCount, SkippedByteCount;


USB_Result USB_CDC_RecieveData (uint8_t* Buffer, uint32_t Length)
{
	USB_Result result = USB_SUCCESS;
	ReceivedByteCount += Length;

#ifdef CONFIG_USB_ENABLE_MAGIC_CODE
	if (Length == 39 && isEqualData(39, Buffer, toBootloaderResetCommand)){
		MTR_Parameter_write(PARAMETER_BOOTSELECT, UsbConnection);
		NVIC_SystemReset();
	}
#endif
	Usb_userRecieveFunction(Buffer, Length);

	SentByteCount += Length;

	if (result != USB_SUCCESS){
	// If data cannot be sent now, it will await nearest possibility
	// (see USB_CDC_DataSent)
		PendingDataLength = Length;
	}

	return result;
}

/* USB Device layer setup and powering on */
USB_Result Mtr_Usb_cdc_init()
{
	/* CDC layer initialization */
	USB_CDC_Init((uint8_t*) &Buffer, 1, SET);

	/* Set connection parameters */
	LineCoding.dwDTERate = 921600 * 2;
	LineCoding.bCharFormat = 0;
	LineCoding.bParityType = 0;
	LineCoding.bDataBits = 8;

	/* Enables the CPU_CLK clock on USB */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_USB, ENABLE);

	/* Device layer initialization */
	USB_Clock_InitStruct.USB_USBC1_Source = USB_C1HSEdiv2;
	USB_Clock_InitStruct.USB_PLLUSBMUL    = USB_PLLUSBMUL6;

	USB_DeviceBUSParam.MODE  = USB_SC_SCFSP_Full;
	USB_DeviceBUSParam.SPEED = USB_SC_SCFSR_12Mb;
	USB_DeviceBUSParam.PULL  = USB_HSCR_DP_PULLUP_Set;

	USB_DeviceInit(&USB_Clock_InitStruct, &USB_DeviceBUSParam);

	/* Enable all USB interrupts */
	USB_SetSIM(USB_SIS_Msk);

	USB_DevicePowerOn();

	NVIC_EnableIRQ(USB_IRQn);

	return (USB_DEVICE_HANDLE_RESET);
}

USB_Result Mtr_Usb_cdc_reinit()
{
	USB_Reset();
	USB_CDC_Init((uint8_t*) &Buffer, 1, SET);
	/* Set pulls and Device mode */
	USB_SetHSCR(USB_HSCR_HOST_MODE_Device);
	USB_SetHSCR(USB_DeviceBUSParam.PULL);

	/* Set speed, polarity and enable end points */
	USB_SetSC(USB_DeviceBUSParam.SPEED | USB_DeviceBUSParam.MODE | USB_SC_SCGEN_Set);
	/* Setup EP0 */

	USB_EP_Init(USB_EP0, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
	USB_EP_setSetupHandler(USB_EP0, &USB_CurrentSetupPacket, USB_DEVICE_HANDLE_SETUP);

	USB_DeviceContext.USB_DeviceState = USB_DEV_STATE_UNKNOWN;
	USB_DeviceContext.Address = 0;

	/* Enable all USB interrupts */

	USB_SetSIM(USB_SIS_Msk);
	USB_DevicePowerOn();

	return (USB_DEVICE_HANDLE_RESET);
}


/* USB_CDC_HANDLE_DATA_SENT implementation - sending of pending data */
USB_Result USB_CDC_DataSent(void)
{
  USB_Result result = USB_SUCCESS;

  if (PendingDataLength)
  {
	result = USB_CDC_SendData(Buffer, PendingDataLength);
#ifdef USB_DEBUG_PROTO
	if (result == USB_SUCCESS) {
		SentByteCount += PendingDataLength;
	}
	else {
		SkippedByteCount += PendingDataLength;
	}
#endif /* USB_DEBUG_PROTO */
	PendingDataLength = 0;
	USB_CDC_ReceiveStart();
  }
  return USB_SUCCESS;
}


#ifdef USB_CDC_LINE_CODING_SUPPORTED

/* USB_CDC_HANDLE_GET_LINE_CODING implementation example */
USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX, USB_CDC_LineCoding_TypeDef* DATA)
{
	assert_param(DATA);
	if (wINDEX != 0) {
	/* Invalid interface */
		return USB_ERR_INV_REQ;
	}

	/* Just store received settings */
	*DATA = LineCoding;
	return USB_SUCCESS;
}

/* USB_CDC_HANDLE_SET_LINE_CODING implementation example */
USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX, const USB_CDC_LineCoding_TypeDef* DATA)
{
	assert_param(DATA);
	if (wINDEX != 0){
	/* Invalid interface */
		return USB_ERR_INV_REQ;
	}

	/* Just send back settings stored earlier */
	LineCoding = *DATA;
	return USB_SUCCESS;
}

#endif /* USB_CDC_LINE_CODING_SUPPORTED */

#ifdef USB_DEBUG_PROTO

/* Overwritten USB_DEVICE_HANDLE_SETUP default handler - to dump received setup packets */
USB_Result USB_DeviceSetupPacket_Debug(USB_EP_TypeDef EPx, const USB_SetupPacket_TypeDef* USB_SetupPacket)
{

#ifdef USB_DEBUG_PROTO
	SetupPackets[SPIndex].packet = *USB_SetupPacket;
	SetupPackets[SPIndex].address = USB_GetSA();
	SPIndex = (SPIndex < USB_DEBUG_NUM_PACKETS ? SPIndex + 1 : 0);
#endif /* USB_DEBUG_PROTO */

	return USB_DeviceSetupPacket(EPx, USB_SetupPacket);
}

#endif /* USB_DEBUG_PROTO */

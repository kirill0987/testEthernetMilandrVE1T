#ifndef MTR_USB_CDC_H_
#define MTR_USB_CDC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "MDR32F9Qx_usb.h"
#include "MDR32F9Qx_usb_CDC.h"
#include "MDR32F9Qx_usb_device.h"


#define BUFFER_LENGTH           512
//#define USB_DEBUG_NUM_PACKETS   1024

USB_Result USB_CDC_RecieveData (uint8_t* Buffer, uint32_t Length);
void __attribute__((weak)) Usb_userRecieveFunction (uint8_t* Buffer, uint32_t Length);


typedef struct {
	USB_SetupPacket_TypeDef packet;
	uint32_t address;
} TDebugInfo;

//static TDebugInfo SetupPackets[USB_DEBUG_NUM_PACKETS];


USB_Result Mtr_Usb_cdc_init();
USB_Result Mtr_Usb_cdc_reinit();

#ifdef __cplusplus
}
#endif

#endif /* MTR_USB_CDC_H_ */

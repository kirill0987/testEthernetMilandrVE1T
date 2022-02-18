#ifndef MTR_ETHERNET_
#define MTR_ETHERNET_

#ifdef __cplusplus
extern "C" {
#endif

#include "MDR32F9Qx_config.h"
#include "cmsis_lib/inc/MDR32F9Qx_eth.h"
#include "mtr_service/mtr_service.h"
#include <stdbool.h>

#define MTR_ETHERNET_AMOUNT 1
#define MTR_ETHERNET_BUFFER_SIZE 4
#define MAC_SIZE 6

typedef struct
{
	uint8_t address[MAC_SIZE];
	uint8_t rAddress[MAC_SIZE];
} Ethernet_MacAddress_t;

typedef struct
{
	MDR_ETHERNET_TypeDef* origin;
	Ethernet_MacAddress_t mac;
} Ethernet_SettingsStruct_t;


typedef struct
{
	uint32_t ethernetBuffer[1514/4];
	int bufferSize;
	bool used;
} Ethernet_DataBuffer;

void MTR_Ethernet_setMac(AvaliableConnectionType_t ethernet,
						 Ethernet_MacAddress_t mac);
Ethernet_MacAddress_t const * MTR_Ethernet_mac(AvaliableConnectionType_t ethernet);
void MTR_Ethernet_init (AvaliableConnectionType_t ethernet);
void MTR_Ethernet_start (AvaliableConnectionType_t ethernet);
void MTR_Ethernet_deInit (AvaliableConnectionType_t ethernet);

void MTR_Ethernet_perform();

#ifdef __cplusplus
}
#endif

#endif

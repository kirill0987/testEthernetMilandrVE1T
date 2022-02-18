#ifndef __TCPIP_H
#define __TCPIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "MDR32F9Qx_config.h"
#include "mtr_ethernet/mtr_ethernet.h"

typedef struct MTR_Tcp_data_t {
	uint32_t size;
	uint8_t* data;
} MTR_Tcp_data_t;

typedef struct MTR_Tcp_port_t {
	uint16_t port;
	void (*dataProcessingFunction) ();
} MTR_Tcp_port_t;

typedef struct Ethernet_IpAddress_t {
	uint32_t address;
	uint32_t rAddress;
} Ethernet_IpAddress_t;

// typedefs
typedef enum {                                   // states of the TCP-state machine
  CLOSED,                                        // according to RFC793 //0
  LISTENING,                                     // 1
  SYN_SENT,                                      // 2
  SYN_RECD,                                      // 3
  ESTABLISHED,                                   // 4
  FIN_WAIT_1,                                    // 5
  FIN_WAIT_2,                                    // 6
  CLOSE_WAIT,                                    // 7
  CLOSING,                                       // 8
  LAST_ACK,                                      // 9
  TIME_WAIT                                      // 10
} TTCPStateMachine;

typedef enum {                                   // type of last frame sent. used
  ARP_REQUEST,                                   // for retransmissions
  TCP_SYN_FRAME,
  TCP_SYN_ACK_FRAME,
  TCP_FIN_FRAME,
  TCP_DATA_FRAME
} TLastFrameSent;


typedef struct {
    uint16_t DestAddr[3];
    uint16_t SrcAddr[3];
    uint16_t FrameType;
    uint16_t HardwareType;
    uint16_t ProtocolType;
    uint16_t HardSizeProtSize;
    uint16_t Opcode;
    uint16_t SenderMacAddr[3];
    uint16_t SenderIPAddr[2];
    uint16_t TargetMACAddr[3];
    uint16_t TargetIPAddr[2];
} sARPHead, *pARPHead;

/* */
typedef struct {
    uint16_t DestAddr[3];
    uint16_t SrcAddr[3];
    uint16_t FrameType;
    uint16_t HardwareType;
    uint16_t ProtocolType;
    uint16_t HardSizeProtSize;
    uint16_t Opcode;
    uint16_t SenderMacAddr[3];
    uint16_t SenderIPAddr[2];
    uint16_t TargetMACAddr[3];
    uint16_t TargetIPAddr[2];
} sEthernetARP_Answer, *pEthernetARP_Answer;

/* */
typedef struct {
    uint16_t DestAddr[3];
    uint16_t SrcAddr[3];
    uint16_t FrameType;
    uint16_t IPVerIHL;
    uint16_t FrameLength;
    uint16_t Identification;
    uint16_t Flags_Offset;
    uint16_t ProtocolType_TTL;
    uint16_t SRC_IP_Frame;
    uint16_t SenderIPAddr[2];
    uint16_t TargetIPAddr[2];
    uint16_t Type_and_Code;
    uint16_t CRS;
    uint16_t Identifier;
    uint16_t Sequence_number;
    uint16_t Data;

} sEthernetIP_Frame, *pEthernetIP_Frame;

/* */
typedef struct {
    uint16_t DestAddr[3];
    uint16_t SrcAddr[3];
    uint16_t FrameType;
    uint16_t IPVerIHL;
    uint16_t FrameLength;
    uint16_t Identification;
    uint16_t Flags_Offset;
    uint16_t ProtocolType_TTL;
    uint16_t SRC_IP_Frame;
    uint16_t SenderIPAddr[2];
    uint16_t TargetIPAddr[2];
    uint16_t SRC_port;
    uint16_t DEST_port;
    uint16_t Sequence_number_HI;
    uint16_t Sequence_number_LO;
    uint16_t Acknowlegement_HI;
    uint16_t Acknowlegement_LO;
    uint16_t Offset_data_and_Flags;
    uint16_t Window_size;
    uint32_t Options;
    uint32_t Data;

} sEthernetTCPIP_Frame, *pEthernetTCPIP_Frame;

/**
 * @brief Init ethernet + tcp stack with early specified parameters (address, gateway, mask)
 * @param port Port for communicating
 * @return Nothing
 */

void MTR_Tcp_init (uint16_t port);

/**
 * @brief Set network address of this device.
 * @param address Address in uint32 format (192.168.1.1 -> 0xC0A80101)
 * @return Nothing
 */
void MTR_Tcp_setAddress (uint32_t address);

/**
 * @brief Set network gateway of this device.
 * @param gateway Gateway address in uint32 format (192.168.1.1 -> 0xC0A80101)
 * @return Nothing
 */
void MTR_Tcp_setGateway (uint32_t gateway);

/**
 * @brief Set network mask of this device.
 * @param mask Network mask in uint32 format (255.255.0.0 -> 0xFFFF0000)
 * @return Nothing
 */
void MTR_Tcp_setMask (uint32_t mask);

void MTR_Tcp_sendData (uint32_t size, uint8_t* data);

// Handlers for incoming frames
void ProcessEthBroadcastFrame(Ethernet_DataBuffer* dataBuffer);
void ProcessEthIAFrame(Ethernet_DataBuffer* dataBuffer);
uint32_t ProcessICMPFrame ( void );
void ProcessTCPFrame(void);

MTR_Tcp_data_t* MTR_Tcp_getReceivedData (void);
MTR_Tcp_port_t* MTR_Tcp_getPortStruct (void);

// prototypes
void MTR_Tcp_doNetworkStuff(MDR_ETHERNET_TypeDef * ETHERNETx);

#ifdef __cplusplus
}
#endif

#endif

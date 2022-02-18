/**
 * @file
 * @brief This file contain function for work with CAN
 *
 * This is full description of the file
 * @ingroup g_MilandrGit__1986VE92__Submodules__Network__Can__mtr_can
 *
 */
#ifndef MTR_CAN_H_
#define MTR_CAN_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "mtr_port/mtr_port.h"
#include "MDR32F9Qx_can.h"

#define CAN_RX_MAX_BUFFER_NUM 20

#define  CAN_STATUS_SHIFT_TX_ERROR_COUNTER  24
#define  CAN_STATUS_SHIFT_RX_ERROR_COUNTER  16
#define  CAN_STATUS_SHIFT_TX_ERROR_OVER     12
#define  CAN_STATUS_SHIFT_RX_ERROR_OVER     11
#define  CAN_STATUS_SHIFT_ERROR_STATUS      9
#define  CAN_STATUS_SHIFT_ID_LOWER          8
#define  CAN_STATUS_SHIFT_ACK_ERROR         7
#define  CAN_STATUS_SHIFT_FRAME_ERROR       6
#define  CAN_STATUS_SHIFT_CRC_ERROR         5
#define  CAN_STATUS_SHIFT_BIT_STUFF_ERROR   4
#define  CAN_STATUS_SHIFT_BIT_ERROR         3
#define  CAN_STATUS_SHIFT_OVER_ERROR        2
#define  CAN_STATUS_SHIFT_TX_READY          1
#define  CAN_STATUS_SHIFT_RX_READY          0

#define baseCanValue 20

#define  CAN_STATUS_MASK_TX_ERROR_COUNTER  0x00FF
#define  CAN_STATUS_MASK_RX_ERROR_COUNTER  0x00FF
#define  CAN_STATUS_MASK_TX_ERROR_OVER     0x0001
#define  CAN_STATUS_MASK_RX_ERROR_OVER     0x0001
#define  CAN_STATUS_MASK_ERROR_STATUS      0x0003
#define  CAN_STATUS_MASK_ID_LOWER          0x0001
#define  CAN_STATUS_MASK_ACK_ERROR         0x0001
#define  CAN_STATUS_MASK_FRAME_ERROR       0x0001
#define  CAN_STATUS_MASK_CRC_ERROR         0x0001
#define  CAN_STATUS_MASK_BIT_STUFF_ERROR   0x0001
#define  CAN_STATUS_MASK_BIT_ERROR         0x0001
#define  CAN_STATUS_MASK_OVER_ERROR        0x0001
#define  CAN_STATUS_MASK_TX_READY          0x0001
#define  CAN_STATUS_MASK_RX_READY          0x0001

typedef enum{
	CAN1 = baseCanValue,
	CAN2
}MTR_Can_Name_t;


typedef enum{
	canSpeed10Kbit = 10,
	canSpeed20Kbit = 20,
	canSpeed50Kbit =  50,
	canSpeed100Kbit = 100,
	canSpeed125Kbit = 125,
	canSpeed250Kbit = 250,
	canSpeed500Kbit = 500,
	canSpeed800Kbit = 800,
	canSpeed1Mbit = 1000
}MTR_Can_Speed_t;


typedef enum{
	STANDART,       // standart (11 bytes)
	EXTENDED	    // extended (29 bytes)
}MTR_Can_Type_t;


typedef struct canStruct{
	MTR_Can_Type_t	    type;
	MDR_CAN_TypeDef*	originName;
	IRQn_Type 			irq;
	MTR_Port_SomePin_t  txPin;
	MTR_Port_SomePin_t  rxPin;
	MTR_Port_SomePin_t  powerPin;
	MTR_Can_Speed_t		speed;
	uint32_t 			pclkMask;
	uint32_t			hclkDiv;
	bool_t				enableSelfConfirmation : 1;
	bool_t				needPowerPin : 1;
}MTR_Can_t;


typedef struct canStatus{
    uint8_t txErrorCounter;
    uint8_t rxErrorCounter;
    bool_t txErrorOver : 1;
    bool_t rxErrorOver : 1;
    uint8_t errorStatus : 2;
    bool_t idLower		: 1;
    bool_t ackError		: 1;
    bool_t frameError	: 1;
    bool_t crcError		: 1;
    bool_t bitStuffError: 1;
    bool_t bitError		: 1;
    bool_t overError	: 1;
    bool_t rxReady		: 1;
    bool_t txReady		: 1;
}MTR_Can_Status_t;


/**
 * @brief Update can speed (can bit rate)
 * @param canName can name
 * @param canSpeed can speed. Enum values
 * @return void
 */
void MTR_Can_setSpeed(MTR_Can_Name_t canName, MTR_Can_Speed_t canSpeed);


/**
 * @brief update can type (extended or standart)
 * @param canName can name
 * @param type type to change (STANDART or EXTENDED)
 * @return void
 */
void MTR_Can_setType(MTR_Can_Name_t canName, MTR_Can_Type_t type);


/**
 * @brief update can tx pin of need can module
 * @param canName can name
 * @param port tx pin port (PORTA...)
 * @param pin tx pin num (PIN0...)
 * @param func tx pin function (ALTER, OVERRID)
 * @return void
 */

void MTR_Can_setTxPin(MTR_Can_Name_t canName, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func);


/**
 * @brief update can rx pin of need can module
 * @param canName can name
 * @param port rx pin port (PORTA...)
 * @param pin rx pin num (PIN0...)
 * @param func rx pin function (ALTER, OVERRID)
 * @return void
 */
void MTR_Can_setRxPin(MTR_Can_Name_t canName, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func);


/**
 * @brief update can power pin of need can module
 * @param canName can name
 * @param port rx pin port (PORTA...)
 * @param pin rx pin num (PIN0...)
 * @return void
 */
void MTR_Can_setPowerPin(MTR_Can_Name_t canName, MTR_Port_Name_t port, MTR_Port_Pin_t pin);


/**
 * @brief enable clock can module
 * @param can module name to start clock
 * @return void
 *
 */
void MTR_Can_enableClock(MTR_Can_Name_t canName);


/*
 * @brief Deinit can function. Reset can parameters to default (zero)
 * @param can name to reset
 * @return void
 *
 */
void MTR_Can_deInit(MTR_Can_Name_t canName);


/**
 * @param can init function.
 * All config procedure is
 * enable clock -> deinit -> init gpio -> init -> initIT -> initFilter -> start
 * Also, this function init tx interrupt.
 * @param canName can name to init.
 * Init use can parameters from main CAN struct.
 * Use set functions to change that parameters
 * @return void
 */

void MTR_Can_init(MTR_Can_Name_t canName);

/**
 * @brief Send standart usual can message.
 * @param canName can name
 * @param id can message ID
 * @param length can message data length
 * @param data pointer to data
 * @return void
 *
 */
void MTR_Can_sendStandartMessage(MTR_Can_Name_t canName, uint16_t id, uint8_t length, uint8_t* data);


/**
 * @brief Send extended usual can message.
 * @param canName can name
 * @param id can message ID
 * @param length can message data length
 * @param data pointer to data
 * @return void
 *
 */
void MTR_Can_sendExtendedMessage(MTR_Can_Name_t canName, uint32_t id, uint8_t length, uint8_t* data);
void MTR_Can_sendExtendedMessageDirect(MTR_Can_Name_t canName, uint32_t id, uint8_t length, uint8_t* data);

/**
 * @brief Send rtr can message.
 * After this function use transmitMessage function
 * @param canName can name
 * @param can type, standart or extended
 * @param id can message ID
 * @return void
 *
 */
void MTR_Can_sendRtrMessage(MTR_Can_Name_t canName, MTR_Can_Type_t type, uint32_t id, uint8_t length);


/**
 *@brief start can perif operating
 *@param canName can name
 *@return void
 */
void MTR_Can_start(MTR_Can_Name_t canName);


/**
 *@brief stop can perif operating
 *@param canName can name
 *@return void
 */
void MTR_Can_stop(MTR_Can_Name_t canName);


/**
 * @brief Init rx interrupt function
 * @param canName can name
 * @param enableRxIT enable/disable interrupt when message incoming
 * @return void
 */
void MTR_Can_initRxIT(MTR_Can_Name_t canName, bool_t enableRxIT);


/**
 * @brief Clear buffer RX interrupt flag. For use in interrupt
 * @param canName canName to clear rx flag
 * @return void
 */

void MTR_Can_ITclearRxFlag(MTR_Can_Name_t canName, uint8_t rxBufferNumber);


/**
 * @brief Clear buffer TX interrupt flag. For use in interrupt
 * @param canName canName
 * @param txBufferNumber buffer number to clear tx flag
 * @return void
 */
void MTR_Can_ITclearTxFlag(MTR_Can_Name_t canName, uint8_t txBufferNumber);


/**
 * @brief Init tx and rx pins for can operating
 * @param canName can name
 * @return void
 */
void MTR_Can_initGPIO(MTR_Can_Name_t canName);


/**
 * @brief  Starts the waiting for the receiving of a message
 * @param canName can name
 * @return void
 */
void MTR_Can_initReceive(MTR_Can_Name_t canName);


/**
 * @brief init hardware can filter. Interrupt occur when (messageId & mask  == filter)
 * @param canName can name
 * @param rxBufferNum buffer number. For receive used 0..9 numbers;
 * @param filter need filter number
 * @param mask need mask number. Can be 0.. 0x1FFFFFFF
 * @return void
 */
void MTR_Can_initFilter(MTR_Can_Name_t canName, uint32_t rxBufferNum, uint32_t filter, uint32_t mask);


/**
 * @brief get current can status from register and fill can status struct.
 * @param canName can name
 * @return pointer to current can status
 */
MTR_Can_Status_t* Mtr_Can_getStatus(MTR_Can_Name_t canName);


/**
 * @brief Return origin Milandr Can struct
 * @param canName can name
 * @return pointer to current can status
 */
MDR_CAN_TypeDef* Mtr_Can_getOrigin(MTR_Can_Name_t canName);


/**
 * @brief Get free can buffer for transmite. Standart function works bad (or just I am stupid)
 * @param canName can name
 * @return pointer to current can status
 */
uint32_t MTR_Can_getEmptyTransferBuffer(MTR_Can_Name_t canName);


/**
 * @brief Check incoming
 * @param canName can name
 * @return pointer to current can status
 */
void MTR_Can_checkMessageForMagicPackage(MTR_Can_Name_t canName, CAN_RxMsgTypeDef* messageToCheck);

MTR_Can_t* MTR_Can_getMainStruct(MTR_Can_Name_t canName);

void MTR_Can_enableAutoConfirmation(MTR_Can_Name_t canName);


#ifdef __cplusplus
}
#endif


#endif /* MTR_CAN_H_ */

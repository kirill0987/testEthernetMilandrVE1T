/**
 * @file 
 * @brief This file contain base function for work with modbus master/slave.
 *
 * If you need modbus, include modbus or modbus_slave lib in addition. This library (modbus_base) is used by these libs.
 * @ingroup g_MilandrGit__1986VE92__Submodules__Network__Modbus__mtr_modbus_base
 * 
 */
#ifndef MTR_MODBUS_BASE_H_
#define MTR_MODBUS_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "config.h"
#include "mtr_service/mtr_service.h"
#ifndef CONFIG_MODBUS_DISABLE_SERIAL
#include "mtr_uart_base/mtr_uart_base.h"
#endif

#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
#include "mtr_tcpip/mtr_tcpip.h"
#include "mtr_ethernet/mtr_ethernet.h"
#endif

#define MODBUS_RTU_PACKET_SIZE_MIN 5
#define MODBUS_RTU_PACKET_SIZE_MAX 128
#define MODBUS_TCP_PACKET_SIZE_MIN 12
#define MODBUS_TCP_PACKET_SIZE_MAX (255 + 5)

#define MODBUS_BYTE_SIZE_DATA_MAX (MODBUS_TCP_PACKET_SIZE_MAX)

#define MODBUS_ADDRESS_BROADCAST 0

#define MODBUS_TCP_PROTOCOL_ID 0
#define MODBUS_TCP_HEADER_SIZE 6

/***** Size of blocks *****/

#define MODBUS_BYTE_SIZE_ADDRESS 1
#define MODBUS_BYTE_SIZE_FUNCTIONAL_CODE 1
#define MODBUS_BYTE_SIZE_START_REGISTER 2
#define MODBUS_BYTE_SIZE_QUANTITY 2
#define MODBUS_BYTE_SIZE_NEXT_BYTES 1
#define MODBUS_BYTE_SIZE_CRC 2
#define MODBUS_BYTE_SIZE_ERROR 1

#define MODBUS_RTU_BYTE_SIZE_CRC 2

/***** Functional codes *****/

#define MODBUS_FUNC_READ_COILS_STATUS 0x1
#define MODBUS_FUNC_READ_DISCR_INPUTS 0x2
#define MODBUS_FUNC_READ_HOLDING_REGS 0x3
#define MODBUS_FUNC_READ_INPUT_REGS 0x4
#define MODBUS_FUNC_WRITE_SINGLE_COIL 0x5
#define MODBUS_FUNC_WRITE_SINGLE_REG 0x6
#define MODBUS_FUNC_WRITE_MULTIPLE_COILS 0xf
#define MODBUS_FUNC_WRITE_MULTIPLE_REGS 0x10

#define MODBUS_RTU_BYTE_SIZE_DATA_MAX (MODBUS_RTU_PACKET_SIZE_MAX - MODBUS_BYTE_SIZE_ADDRESS - MODBUS_BYTE_SIZE_FUNCTIONAL_CODE - MODBUS_RTU_BYTE_SIZE_CRC)

//!Modbus state
typedef enum{

	MODBUS_READY = 0, 			//!< Modbus is ready to build and send new package
	MODBUS_PREPARATION = 1, 	//!< Modbus package build
	MODBUS_DISPATCH = 2, 		//!< Modbus package shipped
	MODBUS_RECEIVED = 3,		//!< Some Modbus package received, processing
	MODBUS_CONFIRM = 4, 		//!< Modbus answer received with no error
	MODBUS_NOANSWERERROR = 5,  	//!< Modbus answer received with CRC error
	MODBUS_TIMEOUT = 6,		    //!< Modbus answer not received
	MODBUS_OTHERERROR = 7,		//!< Something goes really bad
	MODBUS_DATAERROR = 8,
	MODBUS_STANDARTERROR = 9

} MTR_Modbus_status_t;


//!Modbus rtu receive message error. This enum use when error answer generated.
typedef enum{

 	 MODBUS_ERROR_NONE = 0x0,           //!< No error. Good message
 	 MODBUS_ERROR_INVALID_FUNC = 0x1,   //!< Wrong function
     MODBUS_ERROR_INVALID_ADDRESS = 0x2,//!< Wrong register address
 	 MODBUS_ERROR_INVALID_DATA = 0x3,   //!< Wrong data
     MODBUS_ERROR_NONRECOVERABLE = 0x4, //!< Very bad error. Not used
     MODBUS_ERROR_OP_IN_PROGRESS = 0x5, //!< Operation in progress. Not used
     MODBUS_ERROR_BUSY = 0x6,           //!< Slave device is busy. Not used
     MODBUS_ERROR_FUNC = 0x7,           //!< Device cant perform 13 or 14 func. Not used
     MODBUS_ERROR_PARITY = 0x8,         //!< Parity error. Not used
     MODBUS_ERROR_GATEWAY_PATH_UNAVAILABLE = 0xA, //!< Gateway path unavailable. Not used
     MODBUS_ERROR_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 0xB  //!< Gateway target device failed to respond. Not used

} MTR_Modbus_error_t;


//! Modbus modes
typedef enum{

    MODBUS_MODE_SLAVE = 0, //!< Modbus Slave.
    MODBUS_MODE_MASTER = 1 //!< Modbus Master.

} MTR_Modbus_mode_t;


typedef enum {

    HOLDING = 1, //!< Holding register.
    INPUT = 2 //!< Input register.

} MTR_Modbus_registerType_t;


typedef struct {
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
	uint16_t tid;									//!< Transaction id, for TCP
	uint16_t pid;									//!< Protocol id, for TCP
	uint16_t length;								//!< Data length, for TCP
	uint16_t tidSwapped;
	uint16_t pidSwapped;
	uint16_t lengthSwapped;
#endif

    uint8_t address; 			                    //!< Receiver address
    uint8_t func; 	  			                    //!< Need functional code.
    uint8_t data[MODBUS_BYTE_SIZE_DATA_MAX];   	    //!< Data + Start register + Quantity + Nextbyte.
    uint16_t crc;							        //!< CRC hash code, for RTU
    uint16_t dataSize;
    bool_t checked;

} MTR_Modbus_message_t;


typedef struct{

	uint8_t address;
	uint32_t txCounter;
	uint32_t rxCounter;
	uint32_t rxGoodCounter;
	uint32_t noAnswerErrorCounter;
	uint32_t recieveError;
	uint32_t otherError;
	uint32_t statusCounter;
	bool_t lastRequestHasReply;
	bool_t online;

} MTR_Modbus_device_t;


//! Main Modbus structure.
typedef struct{

	AvaliableConnectionType_t connection; 	//!< Any support connection, can be @ref Uart1Connection or another.
	MTR_Modbus_mode_t mode;                 //!< Modbus mode, can be @ref MODBUS_MODE_SLAVE or @ref MODBUS_MODE_MASTER.
    uint8_t address;                        //!< Local modbus address of device
    MTR_Modbus_status_t status;	            //!< Current modbus status
    MTR_Modbus_error_t lastError;           //!< Last occur message

    MTR_Modbus_message_t rxMessage;
    MTR_Modbus_message_t txMessage;

    uint32_t totalRxPackages;       		  //!< Total num of received packages (messages), for stats
    uint32_t totalRxErrorPackages;            //!< Total error received packages (messages), for stats
    uint32_t totalRxGoodPackages;             //!< Total good received packages (messages), for stats
    uint32_t totalTxPackages;                 //!< Total amount of try to send the packages

    void * specificModbusData;

}MTR_Modbus_t;

//! Modbus element of linked list of all modbuses.
typedef struct{

	MTR_Modbus_t* modbus;
	//use void* cause MTR_Modbus_modbusElement cant be used here in C.
	void* nextModbusElement;

}MTR_Modbus_modbusElement;

MTR_Modbus_modbusElement* MTR_Modbus_startModbus();
bool_t MTR_Modbus_setStartModbus(MTR_Modbus_modbusElement* element);

/**
 * @brief Function reset tx message. Dont reset data buffer.
 * @param modbus : Modbus structure to operate.
 * @return : Nothing
 */
void MTR_Modbus_resetTxBuffer(MTR_Modbus_t* modbus);
void MTR_Modbus_resetRxBuffer(MTR_Modbus_t* modbus);

MTR_Modbus_t* MTR_Modbus_getModbusStruct(AvaliableConnectionType_t connection, uint8_t slaveAddress);

void MTR_Modbus_performHandler(AvaliableConnectionType_t connection);
#ifdef CONFIG_MODBUS_ENABLE_ETHERNET
void MTR_Modbus_Ethernet_performHandler();
#endif

#ifndef CONFIG_MODBUS_DISABLE_SERIAL
void MTR_ModbusRtu_uartSwitchToRecieve(AvaliableConnectionType_t uart);
#endif


#ifdef __cplusplus
}
#endif

#endif /*MTR_MODBUS_BASE_H_ */

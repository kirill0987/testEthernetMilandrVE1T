#ifndef MTR_MODBUS_MASTER_H_
#define MTR_MODBUS_MASTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mtr_modbus_base/mtr_modbus_base.h"
#include "mtr_modbus_defines.h"

typedef struct{
	MTR_Modbus_t* parentModbus;
	uint8_t slaveAddress;
	uint16_t registerAddress;
	uint16_t registerValue;
}MTR_Modbus_pollingRegister_t;


typedef struct{
	MTR_Modbus_device_t device[CONFIG_MODBUS_CONNECTED_DEVICE_NUM];
	uint16_t currentDeviceNum;
}MTR_Modbus_connections_t;


typedef struct{
	MTR_Modbus_pollingRegister_t registers[CONFIG_MODBUS_MAX_PULLING_HOLDING_REGISTER_NUM_PER_CONNECTION];
	uint16_t currentPollingRegistersNumber;
}MTR_Modbus_pollingHoldingRegisters_t;


typedef struct{
	MTR_Modbus_pollingRegister_t registers[CONFIG_MODBUS_MAX_PULLING_INPUT_REGISTER_NUM_PER_CONNECTION];
	uint16_t currentPollingRegistersNumber;
}MTR_Modbus_pollingInputRegisters_t;


typedef struct{
    uint32_t totalTxNoAnswerPackages; 	//!< Total amount packages without answer (only master)
    MTR_Modbus_connections_t connections;
    MTR_Modbus_pollingHoldingRegisters_t* pollingHoldingRegisters;
    MTR_Modbus_pollingInputRegisters_t* pollingInputRegisters;
}MTR_Modbus_master_specificData;

/**
 * @brief Init modbus function
 * @param connection : specify connection for modbus.
 * @return void
 */
MTR_Modbus_t* MTR_Modbus_master_init(AvaliableConnectionType_t connection);


MTR_Modbus_device_t* MTR_Modbus_master_getDevice(MTR_Modbus_t* modbus, uint8_t deviceAddress);


errorType MTR_Modbus_master_sendTxMessage(MTR_Modbus_t* modbus);


uint16_t* MTR_Modbus_master_getPollingRegister(MTR_Modbus_t* modbus, uint8_t slaveAddress, MTR_Modbus_registerType_t registerType, uint16_t registerAddress);


/**
 * @brief Initialization request of Modbus Master
 *
 * @param modbus : modbus struct pointer to operate
 * @param function : Functional code of Modbus
 * @param address :  Destination address
 * @param startRegister : Start register address
 * @param quantity : Amount of bits/ bytes
 * @param data : Pointer to data
 * @retval errorType : error
 */
errorType MTR_Modbus_master_initMessage(MTR_Modbus_t* modbus, uint8_t function, uint8_t address, uint16_t startRegister, uint8_t quantity, uint16_t* data);


/**
 * @brief Request function with fc = 0x03. Read Holding Registers.
 *
 * @param modbus : modbus struct pointer to operate
 * @param address :  Destination address
 * @param startRegister : Start register address
 * @param quantity : Amount of bits/ bites
 * @retval errorType : error
 */
errorType MTR_Modbus_master_getHoldingRegisters(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint8_t quantity);


/**
 * @brief Request function with fc = 0x04. Read Input Registers.
 *
 * @param modbus : modbus struct pointer to operate
 * @param address :  Destination address
 * @param startRegister : Start register address
 * @param quantity : Amount of bits/ bites
 * @retval errorType : error
 */
errorType MTR_Modbus_master_getInputRegisters(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint8_t quantity);


/**
 * @brief Request function with fc = 0x06.  Write Single Register.
 *
 * @param uart : UART module to operate
 * @param address :  Destination address
 * @param startRegister : Start register address
 * @param data : Data pointer to write
 * @retval errorType : error
 */
errorType MTR_Modbus_master_setRegister(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint16_t* data);


/**
 * @brief Request function with fc = 0x10. Write Multiple Registers
 *
 * @param uart : UART module to operat
 * @param address :  Destination address
 * @param startRegister : Start register address
 * @param quantity : Amount of bits/ bites
 * @param data : Data pointer to write
 * @retval errorType : error
 */
errorType MTR_Modbus_master_setRegisters(MTR_Modbus_t* modbus, uint8_t address, uint16_t startRegister, uint8_t quantity, uint16_t* data);


/**
 * @brief Modbus package fill function.
 *
 * @param uart : UART module to operate
 * @param address :  Destination address
 * @param startRegister : Start register address
 * @param quantity : number of bits/ bites
 * @param data : Data pointer to write
 * @retval errorType : error
 */
void MTR_Modbus_master_setMessageData(MTR_Modbus_t* modbus, uint8_t function, uint8_t quantity, uint16_t* data);


/**
  * @brief This function read the uart buffer and verify answer.
  *
  * @param uart : UART module to operate
  * @retval void.
  */
void MTR_Modbus_master_answerChecker(MTR_Modbus_t* modbus);


void MTR_Modbus_master_updateRegistersByReply(MTR_Modbus_t* modbus);

void __attribute__((weak)) MTR_Modbus_master_callbackAfterProccess(MTR_Modbus_t* modbus);

#ifdef __cplusplus
}
#endif

#endif /* MTR_MODBUS_H_ */

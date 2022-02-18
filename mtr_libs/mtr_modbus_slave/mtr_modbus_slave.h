#ifndef MTR_MODBUS_SLAVE_H_
#define MTR_MODBUS_SLAVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mtr_modbus_base/mtr_modbus_base.h"


/***** Max coils amount. Set as 0, cause not released *****/
#define MODBUS_RTU_MAX_AMOUNT_COILS 0


/*
 * Register structure
 * address - Modbus register address
 * viriableAddress - var pointer, modbus register bind to this pointer
 */
typedef struct{

	bool_t inited;
	uint16_t address;
	uint16_t* variableAddress;

} MTR_Modbus_slave_registerStruct_t;


/***** Current num of holding and input registers *****/
typedef struct{

	MTR_Modbus_slave_registerStruct_t holdingRegisterMap[CONFIG_MODBUS_MAX_AMOUNT_HOLDING_REGISTERS_PER_SLAVE];
	MTR_Modbus_slave_registerStruct_t inputRegisterMap[CONFIG_MODBUS_MAX_AMOUNT_INPUT_REGISTERS_PER_SLAVE];
	uint16_t currentHoldingRegisterIndex;
	uint16_t currentInputRegisterIndex;

}MTR_Modbus_slave_specificData;


/* Not released modbus functions
 *
 * 	fc 01 02
 *	void MTR_ModbusRtu_slave_getCoils(MTR_Uart_Name_t uart);
 *	fc 05
 *	void MTR_ModbusRtu_slave_setCoil(MTR_Uart_Name_t uart);
 *	fc 15
 *	void MTR_ModbusRtu_slave_setCoils(MTR_Uart_Name_t uart);
 */

MTR_Modbus_t* MTR_Modbus_slave_init(AvaliableConnectionType_t connection, uint8_t address);


errorType MTR_Modbus_slave_sendTxMessage(MTR_Modbus_t* modbus);

/**
 * @brief Function of writing Single Register (fc 0x06).
 * @param modbus modbus to operate.
 * @return  Nothing.
 */
void MTR_Modbus_slave_setSingleHoldingRegister(MTR_Modbus_t* modbus);


/**
 * @brief Function of writing  Registers (fc 0x16).
 * @param modbus modbus to operate.
 * @return Nothing.
 */

void MTR_Modbus_slave_setHoldingRegisters(MTR_Modbus_t* modbus);


/**
 * @brief : Add to holding register map a new register. Return @ref E_OUT_OF_RANGE if num of registers is out of range.
 * If register already exist, old register will be rewrited with new variable address.
 * @param modbus modbus to operate.
 * @param address : Modbus register address.
 * Can be 0 .. 65535.
 * @param variableAddress :  Pointer to var, which will be bind to modbus register.
 * @return error.
 */
errorType MTR_Modbus_slave_addHoldingRegister(MTR_Modbus_t* modbus, uint8_t address, uint16_t* variableAddress);


/*
 * @brief : Add to input register map new register. Return E_OUT_OF_RANGE if num of registers is out of range.
 * If register already exist, old register will be rewrite with new variable address.
 * @param modbus modbus to operate.
 * @param address Modbus register address.
 * Can be 0 .. 65535.
 * @param variableAddress Pointer to var, which will be bind to modbus register.
 * @return errorType error.
 *
 */
errorType MTR_Modbus_slave_addInputRegister(MTR_Modbus_t* modbus, uint8_t address, uint16_t* variableAddress);


/*
 * @brief : Get the variable address of register from holding register map. Return zero address if register was no found.
 * @param modbus modbus to operate.
 * @param address Modbus register address.
 * Can be 0 .. 65535.
 * @return Pointer to var.
 *
 */
__attribute__((always_inline)) uint16_t* MTR_Modbus_slave_getLocalHoldingRegister(MTR_Modbus_t* modbus, uint16_t address);


/*
 * @brief : Get the variable address of register from input register map. Return zero address if register was no found.
 * @param modbus modbus to operate.
 * @param address Modbus register address.
 * Can be 0 .. 65535.
 * @return Pointer to var.
 *
 */
__attribute__((always_inline)) uint16_t* MTR_Modbus_slave_getLocalInputRegister(MTR_Modbus_t* modbus, uint16_t address);


void __attribute__((weak)) MTR_Modbus_slave_callbackAfterProccess(MTR_Modbus_t* modbus);

void __attribute__((weak)) MTR_Modbus_slave_callbackBeforeProccess(MTR_Modbus_t* modbus);

#ifdef __cplusplus
}
#endif

#endif /* MTR_MODBUS_SLAVE_H_ */

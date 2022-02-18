#ifndef MTR_EEPROM_H_
#define MTR_EEPROM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "MDR32F9Qx_eeprom.h"
#include "config.h"

#if defined (USE_MDR1986VE1T)
#define FLASH_END_ADDRESS 0x00020000
#define FLASH_PARAMETERS_START_ADDRESS ((uint32_t) (0x0001F000))
#define FLASH_PARAMETERS_AMOUNT (1024)
#endif

#if defined ( USE_MDR1986VE9x )
#define FLASH_END_ADDRESS 0x08020000
#define FLASH_PARAMETERS_START_ADDRESS ((uint32_t) (0x0801F000))
#define FLASH_PARAMETERS_AMOUNT (1024)
#endif

#ifndef CONFIG_EEPROM_BUFFER_BYTE_SIZE
#define CONFIG_EEPROM_BUFFER_BYTE_SIZE 4096
#endif

#define	PARAMETER_BOOTSELECT 1
#define PARAMETER_ID 2
#define	PARAMETER_FIRMWARE_SIZE 3
#define PARAMETER_FIRMWARE_CRC 4
#define	PARAMETER_FIRMWARE_VERSION 5
#define PARAMETER_FIRMWARE_TRIES 6
#define PARAMETER_CONNECTION_RXPINPORT 7
#define PARAMETER_CONNECTION_TXPINPORT 8
#define PARAMETER_CONNECTION_ENPINPORT 9
#define PARAMETER_CONNECTION_SPEED_AND_PARITY 10
#define PARAMETER_CONNECTION_EXTRAPINPORT 11
#define PARAMETER_START_DELAY 12
#define PARAMETER_DEVICE_TEST_LED_PORTPIN 13

#define PARAMETER_START_FIRMWARE_ADDRESS 122
#define PARAMETER_EXPERTMODE 123
#define PARAMETER_PROGRAMM_PROFILE 125

void* MTR_Eeprom_getBuffer();
void MTR_Eeprom_init();
uint8_t MTR_Eeprom_readByte(uint32_t address);
uint32_t MTR_Eeprom_readWord(uint32_t address);
uint16_t MTR_Eeprom_readHalfWord(uint32_t address);
void MTR_Eeprom_writeByte(uint32_t address, uint8_t byte);
void MTR_Eeprom_writeWord(uint32_t address, uint32_t word);
void MTR_Eeprom_writeHalfWord(uint32_t address, uint16_t halfWord);
__RAMFUNC void MTR_Eeprom_erasePage(uint32_t address);

__RAMFUNC void MTR_Parameter_write(uint16_t parameter, uint32_t value);

__RAMFUNC uint32_t MTR_Parameter_read(uint16_t parameter);

#ifdef __cplusplus
}
#endif

#endif /* MTR_EEPROM_H_ */

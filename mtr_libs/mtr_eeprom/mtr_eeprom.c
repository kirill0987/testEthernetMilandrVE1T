#include "mtr_eeprom/mtr_eeprom.h"
#include "MDR32F9Qx_rst_clk.h"
#include <malloc.h>

uint8_t eepromBufferData[CONFIG_EEPROM_BUFFER_BYTE_SIZE];
void* eepromBuffer = &eepromBufferData;

__RAMFUNC void MTR_Parameter_reWrite(uint16_t parameter, uint32_t value);

void* MTR_Eeprom_getBuffer(){
	return eepromBuffer;
}

void MTR_Eeprom_init(){

    RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);

    if (CONFIG_CURRENT_CLOCK_HZ < 25000000){
    	EEPROM_SetLatency(EEPROM_Latency_0);
    }
    else if (CONFIG_CURRENT_CLOCK_HZ < 50000000){
		EEPROM_SetLatency(EEPROM_Latency_1);
	}
    else if (CONFIG_CURRENT_CLOCK_HZ < 75000000){
    	EEPROM_SetLatency(EEPROM_Latency_2);
    }
    else if (CONFIG_CURRENT_CLOCK_HZ < 100000000){
    	EEPROM_SetLatency(EEPROM_Latency_3);
    }
    else if (CONFIG_CURRENT_CLOCK_HZ < 125000000){
    	EEPROM_SetLatency(EEPROM_Latency_4);
    }
    else {
    	EEPROM_SetLatency(EEPROM_Latency_7);
    }
}

uint8_t MTR_Eeprom_readByte(uint32_t address){
    return EEPROM_ReadByte(address, EEPROM_Main_Bank_Select);
}

uint32_t MTR_Eeprom_readWord(uint32_t address){
    return EEPROM_ReadWord(address, EEPROM_Main_Bank_Select);
}

uint16_t MTR_Eeprom_readHalfWord(uint32_t address){
    return EEPROM_ReadHalfWord(address, EEPROM_Main_Bank_Select);
}

void MTR_Eeprom_writeByte(uint32_t address, uint8_t byte){
    EEPROM_ProgramByte(address, EEPROM_Main_Bank_Select, (uint32_t) byte);
}

void MTR_Eeprom_writeWord(uint32_t address, uint32_t word){
    EEPROM_ProgramWord(address, EEPROM_Main_Bank_Select, word);
}

void MTR_Eeprom_writeHalfWord(uint32_t address, uint16_t halfWord){
    EEPROM_ProgramHalfWord(address, EEPROM_Main_Bank_Select, (uint32_t) halfWord);
}

__RAMFUNC void MTR_Eeprom_erasePage(uint32_t address){
    EEPROM_ErasePage(address, EEPROM_Main_Bank_Select);
}

__RAMFUNC void MTR_Parameter_write(uint16_t parameter, uint32_t value){

	if (!(((EEPROM_ReadWord(FLASH_PARAMETERS_START_ADDRESS + (parameter*4), EEPROM_Main_Bank_Select)) & value )== value)) {
		//cant write new value without erase
		MTR_Parameter_reWrite(parameter, value);
	}
	else {
		//can write new value
		EEPROM_ProgramWord(FLASH_PARAMETERS_START_ADDRESS + (parameter*4), EEPROM_Main_Bank_Select, value);
	}
}

__RAMFUNC void MTR_Parameter_reWrite(uint16_t parameter, uint32_t value){

	uint32_t* buffer = eepromBuffer;

	for(uint16_t i = 0; i < FLASH_PARAMETERS_AMOUNT; i++){
		buffer[i] = EEPROM_ReadWord(FLASH_PARAMETERS_START_ADDRESS + (i*4), EEPROM_Main_Bank_Select);
		if (i == parameter){
			buffer[i] = value;
		}
	}

	MTR_Eeprom_erasePage(FLASH_PARAMETERS_START_ADDRESS);

	for (uint16_t i = 0; i < FLASH_PARAMETERS_AMOUNT; i++){
		if (buffer[i] != 0xFFFF){
			EEPROM_ProgramWord(FLASH_PARAMETERS_START_ADDRESS + (i*4), EEPROM_Main_Bank_Select, buffer[i]);
		}
	}
}

__RAMFUNC uint32_t MTR_Parameter_read(uint16_t parameter){
	uint32_t address = FLASH_PARAMETERS_START_ADDRESS + (parameter*4);
	return (EEPROM_ReadWord(address, EEPROM_Main_Bank_Select));
}

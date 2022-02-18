#include "mtr_crc/mtr_crc.h"

uint16_t MTR_Crc_getCrc16Modbus(uint8_t *data, uint32_t length){

    uint16_t crc = 0xFFFF;

    while (length--) {
        crc = (crc >> 8) ^ crc16Table[(crc & 0xFF) ^ *data++];
    }
    return crc;
}

uint8_t MTR_Crc_getCrc8(uint8_t* data, uint32_t length)
{
	uint16_t crc = 0xFF;

    while (length--) {
        crc = crc8Table[crc ^ *data++];
    }

    return crc;
}

uint16_t MTR_Crc_getCrc16(uint8_t* data, uint32_t length)
{
    uint16_t crc = 0x0000;

    while (length--) {
        crc = (crc >> 8) ^ crc16Table[(crc & 0xFF) ^ *data++];
    }
    return crc;
}

uint32_t MTR_Crc_getCrc32(uint8_t* data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    while (length) {
        crc = (crc >> 8) ^ crc32Table[(crc ^ *data++) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}

#ifndef MTR_TOOLS_H_
#define MTR_TOOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "config.h"

#define RST_CLK_PER_CLOCK_RST_CLK (1<<4)
#define RST_CLK_PER_CLOCK_BKP (1<<27)

#define getBit(x, y) ((x >> y) & 1)

#define RTC_PRESCALER 32768
#define RTC_CALIBRATION 0

typedef enum {
    reset = 0,
    set = !reset
} bitStatus_t;

typedef enum {
    disable = 0,
    enable = !disable
} status_t;

#ifdef __cplusplus
using bool_t = bool;
#else
typedef enum{
    false = 0,
    true = !false
}bool_t;
#endif

typedef struct {
    char letter;
    uint8_t num;
} hexCodes_t;

typedef enum {
    Uart1Connection = 10,
    Uart2Connection = 11,
    Can1Connection = 20,
    Can2Connection = 21,
    SpiConnection = 100,
    UsbConnection = 110,
    I2CConnection = 120,
    Ethernet = 200,
    Unknown = 255,
    NoConnection = 0x1FFFFFFF
} AvaliableConnectionType_t;

typedef enum {
    E_NO_ERROR,
    E_NULL_POINTER,
    E_INVALID_VALUE,
    E_OUT_OF_RANGE,
    E_BUSY,
    E_IO_ERROR,
    E_OUT_OF_MEMORY,
    E_NOT_IMPLEMENTED,
    E_CRC,
    E_STATE,
    E_TIMEOUT
} errorType;

extern uint8_t toBootloaderResetCommand[39];

void initDevice();
uint32_t checkParameter(uint32_t value, uint32_t minValue, uint32_t maxValue,
        uint32_t defaultValue);
bool_t isEqualData(uint32_t byteSize, uint8_t* pBuffer1, uint8_t* pBuffer2);
void delay_ms(uint16_t delay_ms);
void delay_micros(uint16_t delay_timeout);
void initClock();
int initClockForce();
void enableTacting();

void write32To8Bits(uint8_t* to, uint32_t what);
uint32_t get32From8Bits(uint8_t* from);
uint16_t get16From8Bits(uint8_t* from);
void set8From32Bits(uint8_t* to, uint32_t* from);
void set8From16Bits(uint8_t* to, uint16_t* from);


//Type data motor float 1/2/3/4
//----Motor float 1----
uint16_t motorValue1(float value);
float deMotorValue1(uint16_t value);
uint16_t motorValue2(float value);
float deMotorValue2(uint16_t value);
uint16_t motorValue3(float value);
float deMotorValue3(uint16_t value);
uint16_t motorValue4(float value);
float deMotorValue4(uint16_t value);


void disableInterrupts();

/**	@brief Return need bytes to safe bites size of parameter bites
 * 	1bites = 1 byte;
 * 	...
 * 	8bites = 1 byte;
 * 	9bites = 2 byte;
 * 	10 bites = 2 byte;
 * 	@param bites bite size
 * 	@return need bytes size
 *
 */
uint16_t bitesToBytes(uint16_t bites);

void byteShiftLeft(uint8_t* where, uint16_t size, uint16_t shiftNum);

uint32_t swapBytes32(uint32_t wordToSwap);
uint16_t swapBytes16(uint16_t wordToSwap);

/**@brief Set bytes to zero.
 * @param toZeroPointer pointer to data
 * @param size byte size to zero set
 * return void
 */
void toZeroByteFunction(void* toZeroPointer, uint32_t size);
void toZeroHalfWordFunction(void* toZeroPointer, uint32_t size);
void toZeroWordFunction(void* toZeroPointer, uint32_t size);

void charToHex(uint8_t* numsFrom, uint16_t length, uint8_t* to);
void hexToChar(uint8_t* numsFrom, uint16_t length, uint8_t* to);

uint16_t frvToUint(float doubleNum, uint8_t resolutionPow, uint16_t offset);
float uintToFrv(uint16_t uintNum, uint8_t resolutionPow, uint16_t offset);

/**@brief Try to found symbol position in data
 * @param symbol byte try to found
 * @param data pointer to search
 * return position, if not found in next 255 bytes return -1
 */
int foundNextBytePosition(uint8_t symbol, uint8_t* data);

uint32_t getDeviceId();
void setDeviceId(uint32_t newDeviceId);

#ifdef AUTORELOAD
void HardFault_Handler(void);
void Reset_Handler(void);
void NMI_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void WWDG_IRQHandler(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MTR_TOOLS_H_ */

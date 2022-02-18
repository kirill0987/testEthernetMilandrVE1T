#include "mtr_service/mtr_service.h"
#include "mtr_port/mtr_port.h"
#include "mtr_eeprom/mtr_eeprom.h"
#include "mtr_crc/mtr_crc.h"
#include "MDR32F9Qx_rst_clk.h"

#include <string.h>
#include <malloc.h>


uint32_t deviceID;
uint8_t toBootloaderResetCommand[39];
uint8_t bootloaderMagicData[] = "IREALLYWANTTOGOTOBOOTLOADERNOW";

volatile uint32_t delay_dec = 0;

hexCodes_t hexCodes[] = { { '0', 0 }, { '1', 1 }, { '2', 2 }, { '3', 3 }, { '4',
        4 }, { '5', 5 }, { '6', 6 }, { '7', 7 }, { '8', 8 }, { '9', 9 }, { 'A',
        10 }, { 'B', 11 }, { 'C', 12 }, { 'D', 13 }, { 'E', 14 }, { 'F', 15 }, {
        'a', 10 }, { 'b', 11 }, { 'c', 12 }, { 'd', 13 }, { 'e', 14 },
        { 'f', 15 }, };

#ifndef USE_FREERTOS

void SysTick_Handler()
{
    if (delay_dec){
        delay_dec--;
    }
    else {
        SysTick->CTRL = 0;
    }
}

void delay_ms(uint16_t delay_timeout)
{
    if ((CONFIG_CURRENT_CLOCK_HZ / 1000) - 1 * delay_timeout < 0xFFFFFF)
    {
        //not interrupt
        SysTick->LOAD = (( ((uint32_t) CONFIG_CURRENT_CLOCK_HZ) / 1000) - 1) * delay_timeout; // 64MHz = 64000, 8MHz = 8000

        SysTick->CTRL = (1 << SysTick_CTRL_CLKSOURCE_Pos) |
                (1 << SysTick_CTRL_ENABLE_Pos);

        while (!(SysTick->CTRL & (1 << SysTick_CTRL_COUNTFLAG_Pos))) {

        };

        SysTick->CTRL = 0;

    }
    else
    {
        //interrupt
        NVIC_EnableIRQ(SysTick_IRQn);
        delay_dec = delay_timeout;

        SysTick->LOAD = (CONFIG_CURRENT_CLOCK_HZ / 1000) - 1; // 64MHz = 64000, 8MHz = 8000

        SysTick->CTRL = (1 << SysTick_CTRL_CLKSOURCE_Pos) |
                (1 << SysTick_CTRL_ENABLE_Pos) | (1 << SysTick_CTRL_TICKINT_Pos);

        while (delay_dec > 0) {
        };

        SysTick->CTRL = 0;
        NVIC_DisableIRQ(SysTick_IRQn);
    }
}

void delay_micros(uint16_t delay_timeout)
{
    if ((CONFIG_CURRENT_CLOCK_HZ / 1000000) - 1 * delay_timeout < 0xFFFFFF){

        //not interrupt version

        SysTick->LOAD = (((uint32_t) CONFIG_CURRENT_CLOCK_HZ / 1000000) - 1) * delay_timeout; // 64MHz = 64000, 8MHz = 8000

        SysTick->CTRL = (1 << SysTick_CTRL_CLKSOURCE_Pos) |
                (1 << SysTick_CTRL_ENABLE_Pos);

        while (!(SysTick->CTRL & (1 << SysTick_CTRL_COUNTFLAG_Pos))) {
        };

        SysTick->CTRL = 0;

    }
    else
    {
        //interrupt version
        NVIC_EnableIRQ(SysTick_IRQn);

        delay_dec = delay_timeout;

        SysTick->LOAD = (CONFIG_CURRENT_CLOCK_HZ / 1000000) - 1; // 64MHz = 64000, 8MHz = 8000

        SysTick->CTRL = (1 << SysTick_CTRL_CLKSOURCE_Pos) |
                (1 << SysTick_CTRL_ENABLE_Pos) | (1 << SysTick_CTRL_TICKINT_Pos);


        while (delay_dec > 0) {

        };

        SysTick->CTRL = 0;
        NVIC_DisableIRQ(SysTick_IRQn);
    }
}

#endif


//Type data motor float 1/2/3/4
//----Motor float 1----
uint16_t motorValue1(float value) {
	return (uint16_t) ((value + 327.67) / 0.01);
}

float deMotorValue1(uint16_t value) {
	return (float) ((value * 0.01) - 327.67);
}

//----Motor float 2----
uint16_t motorValue2(float value) {
	return (uint16_t) ((value + 3276.7) / 0.1);
}

float deMotorValue2(uint16_t value) {
	return (float) ((value * 0.1) - 3276.7);
}

//----Motor float 3----
uint16_t motorValue3(float value) {
	return (uint16_t) (value / 0.1);
}

float deMotorValue3(uint16_t value) {
	return (float) (value * 0.1);
}

//----Motor float 4----
uint16_t motorValue4(float value) {
	return (uint16_t)(value / 0.01);
}

float deMotorValue4(uint16_t value) {
	return (float) (value * 0.01);
}


uint32_t checkParameter(uint32_t value, uint32_t minValue, uint32_t maxValue,
        uint32_t defaultValue)
{

    if (value > maxValue) {
        return defaultValue;
    }

    if (value < minValue) {
        return defaultValue;
    }

    if (value == 0xFFFFFFFF) {
        return defaultValue;
    }

    return value;
}

void disableInterrupts()
{

    NVIC->ICPR[0] = 0xFFFFFFFF;
    NVIC->ICER[0] = 0xFFFFFFFF;
}

uint16_t bitesToBytes(uint16_t bites)
{

    uint16_t bytes = 0;

    if (bites < 8) {
        bytes = 1;
    }
    else {
        while (bites > 7) {
            bytes += 1;
            bites -= 8;
        }

        if (bites != 0) {
            bytes += 1;
        }
    }

    return bytes;
}

bool_t isEqualData(uint32_t byteSize, uint8_t *pBuffer1, uint8_t *pBuffer2)
{
    uint32_t i;

    for (i = 0; i < byteSize; i++) {
        if (*pBuffer1++ != *pBuffer2++) {
            return (false);
        }
    }

    return (true);
}

void initClock()
{
    RST_CLK_DeInit();
    RST_CLK_HSEconfig(RST_CLK_HSE_ON);

    while (RST_CLK_HSEstatus() != SUCCESS) {
    }

    RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv2,
    CONFIG_RST_CLK_PLL_MULTIPL);
    RST_CLK_CPU_PLLcmd(ENABLE);

    while (RST_CLK_CPU_PLLstatus() != SUCCESS)
        ;

    RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

    RST_CLK_CPU_PLLuse(ENABLE);

    RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

    //switch off perif tacting)
    RST_CLK_PCLKcmd(0xFFFFFFEF, DISABLE);

    // Switch tacting source of CPU
    RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);
}

int initClockForce()
{
    int quarzPllInited = 2;

    RST_CLK_DeInit();
    RST_CLK_HSEconfig(RST_CLK_HSE_ON);

    for (uint32_t i = 65000; i; i--) {
    }

    if (RST_CLK_HSEstatus() == SUCCESS) {
        quarzPllInited = 1;

        RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv2, CONFIG_RST_CLK_PLL_MULTIPL);
        RST_CLK_CPU_PLLcmd(ENABLE);

        for (uint32_t i = 65000; i; i--) {
        }

        if (RST_CLK_CPU_PLLstatus() == SUCCESS) {

            RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);
            RST_CLK_CPU_PLLuse(ENABLE);
            quarzPllInited = 0;
        }

        RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);
        //switch off perif tacting
        RST_CLK_PCLKcmd(0xFFFFFFEF, DISABLE);
    }

    return quarzPllInited;
}

inline void enableTacting()
{

    RST_CLK_PCLKcmd((RST_CLK_PCLK_RST_CLK), ENABLE);
    RST_CLK_PCLKcmd((RST_CLK_PCLK_SSP1 | RST_CLK_PCLK_SSP2), ENABLE);
}

inline void write32To8Bits(uint8_t *to, uint32_t what)
{

    to[0] = (uint8_t) (what >> 24);
    to[1] = (uint8_t) (what >> 16);
    to[2] = (uint8_t) (what >> 8);
    to[3] = (uint8_t) (what >> 0);
}

inline uint32_t get32From8Bits(uint8_t *from)
{

    uint32_t temp = 0;
    temp |= 0x000000FF & from[3];
    temp |= 0x0000FF00 & (from[2] << 8);
    temp |= 0x00FF0000 & (from[1] << 16);
    temp |= 0xFF000000 & (from[0] << 24);
    return temp;
}

inline uint16_t get16From8Bits(uint8_t *from)
{

    uint16_t temp = 0;

    temp = from[1] | (((uint16_t) from[0]) << 8);
    return temp;
}

inline void set8From32Bits(uint8_t *to, uint32_t *from)
{

    *to = (uint8_t) (*from >> 24);
    *(to + 1) = (uint8_t) (*from >> 16);
    *(to + 2) = (uint8_t) (*from >> 8);
    *(to + 3) = (uint8_t) *from;
}

inline void set8From16Bits(uint8_t *to, uint16_t *from)
{

    *to = (uint8_t) (*from >> 8);
    *(to + 1) = (uint8_t) *from;
}

//Array shift
void byteShiftLeft(uint8_t *where, uint16_t size, uint16_t shiftNum)
{

    for (uint16_t i = 0; i < (size - shiftNum); i++) {
        *(where + i) = *(where + i + shiftNum);
    }

    for (uint16_t i = shiftNum; i < size; i++) {
        *(where + i) = 0;
    }
}

inline uint32_t swapBytes32(uint32_t wordToSwap)
{
    return ((wordToSwap >> 24) | (wordToSwap & 0xFF0000) >> 8
            | (wordToSwap & 0xFF00) << 8 | (wordToSwap & 0xFF) << 24);
}

inline uint16_t swapBytes16(uint16_t wordToSwap)
{
    return ((wordToSwap & 0xFF00) >> 8 | (wordToSwap & 0xFF) << 8);
}

inline void toZeroByteFunction(void *toZeroPointer, uint32_t size)
{
    memset(toZeroPointer, 0, size);
}

inline void toZeroHalfWordFunction(void *toZeroPointer, uint32_t size)
{
    memset(toZeroPointer, 0, (size_t) size * 2);
}

inline void toZeroWordFunction(void *toZeroPointer, uint32_t size)
{
    memset(toZeroPointer, 0, (size_t) size * 4);
}

inline void charToHex(uint8_t *numsFrom, uint16_t length, uint8_t *to)
{
    char charTemp[2];
    for (int16_t i = 0; i < length; i += 1) {

        charTemp[0] = *(numsFrom + 2 * i);
        charTemp[1] = *(numsFrom + 2 * i + 1);

        uint8_t temp = 0;

        for (uint8_t i = 0; i < 22; i++) {

            if (charTemp[0] == hexCodes[i].letter) {
                temp |= temp + (hexCodes[i].num * 16);
            }

            if (charTemp[1] == hexCodes[i].letter) {
                temp |= temp + hexCodes[i].num;
            }
        }

        *(to + i) = temp;
    }
}

inline void hexToChar(uint8_t *numsFrom, uint16_t length, uint8_t *to)
{
    for (uint16_t i = 0; i < length; i += 1) {
        uint8_t num = *(numsFrom + i);
        to[2 * i] = hexCodes[num >> 4].letter;
        to[2 * i + 1] = hexCodes[num & 0xf].letter;
    }
}

uint16_t frvToUint(float doubleNum, uint8_t resolutionPow, uint16_t offset)
{

    uint32_t temp = 1;
    while (resolutionPow--) {
        temp *= 10;
    }

    return ((uint16_t) ((double) doubleNum * temp) + offset);
}

float uintToFrv(uint16_t uintNum, uint8_t resolutionPow, uint16_t offset)
{

    uint32_t temp = 1;
    while (resolutionPow--) {
        temp *= 10;
    }

    return ((float) ((int) uintNum - offset) / temp);
}

int foundNextBytePosition(uint8_t symbol, uint8_t *data)
{

    int i = -1;

    while (*(data++)) {
        if (++i == symbol) {
            break;
        }
    }

    return i;
}

void initDevice()
{

    __disable_irq();

#ifdef USE_MDR1986VE9x
    __disable_fault_irq();
#endif

    disableInterrupts();
    initClock();
    enableTacting();
    disableInterrupts();
    MTR_Eeprom_init();
    deviceID = (0x0FFFFFFF & MTR_Parameter_read(PARAMETER_ID)) | (1 << 28);

    //Build magic message

    set8From32Bits(toBootloaderResetCommand, &deviceID);
    toBootloaderResetCommand[4] = 0xFF;
    toBootloaderResetCommand[5] = 0xFF;
    toBootloaderResetCommand[6] = 0xFF;

    memcpy(toBootloaderResetCommand + 7, bootloaderMagicData,
            sizeof(bootloaderMagicData) - 1);

    uint16_t crcToBootloaderMessage = MTR_Crc_getCrc16(toBootloaderResetCommand,
            37);
    set8From16Bits(
            toBootloaderResetCommand + (sizeof(bootloaderMagicData) + 7 - 1),
            &crcToBootloaderMessage);

    //Mtr_Usb_cdc_init();
    //NVIC_SetPriority(USB_IRQn, 4);

}

uint32_t getDeviceId()
{
    return 0x1FFFFFFF & deviceID;
}

void setDeviceId(uint32_t newDeviceId)
{
    deviceID = 0x1FFFFFFF & newDeviceId;
}

#ifdef AUTORESET
void HardFault_Handler(void)
{
    NVIC_SystemReset();
}

void NMI_Handler(void)
{
    NVIC_SystemReset();
}

void MemManage_Handler(void)
{
    NVIC_SystemReset();
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    NVIC_SystemReset();
}

void WWDG_IRQHandler(void)
{
    NVIC_SystemReset();
}
#endif

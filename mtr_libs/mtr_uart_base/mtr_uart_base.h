#ifndef MTR_UART_BASE_H_
#define MTR_UART_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "config.h"
#include "mtr_uart_defines.h"

#include "mtr_port/mtr_port.h"
#include "mtr_service/mtr_service.h"
#include "mtr_timer/mtr_timer.h"
#include "mtr_service/mtr_service.h"

#include "MDR32F9Qx_uart.h"

#define UART_AMOUNT 2
#define UART_TIMER_NUMS_TOTAL_AMOUNT (8)
#define baseUartValue 10

#define uartTimerRepeatesNotCounting 0
#define uartTimerRepeatesSwitchToReadAfterTransmit 1
#define uartTimerRepeatesProccessingRecievedMessage 2
#define uartTimerRepeatesUserTimeout1 3
#define uartTimerRepeatesUserTimeout2 4
#define uartTimerRepeatesUserTimeout3 5
#define uartTimerRepeatesUserTimeout4 6
#define uartTimerRepeatesStartTransmit 7

static const uint8_t crSymbol = 13;
static const uint8_t bellSymbol = 7;

typedef enum
{
	UART = 0,
	RS485 = 1,
} MTR_Uart_Type_t;


typedef struct
{
	uint8_t buffer[CONFIG_UART_BUFFER_BYTE_SIZE]; 	// RX buffer
	uint16_t amount;
	bool_t free;

} MTR_Uart_BufferStruct_t;

typedef struct
{
	MTR_Uart_BufferStruct_t b;
	uint16_t readPos;
} MTR_Uart_TxBufferStruct_t;

typedef enum
{
	NONE = 0,  	// none symbol
	LFCR,     	// line feed and carriage return (0x0A, 0x0D)
	CRLF,     	// carriage return and line feed (0x0D,0x0A)
	LF,        	// line feed (0x0A)
	CR,       	// carriage return (0x0D)
	COLON,		// ":" 	0x3A
	SP,	    	// " " 	0x20
	DOT			// "." 	0x2E
} MTR_Uart_Lastbyte_t;

typedef enum
{
	StandartUsual,
	StandartRtr,
	ExtendedUsual,
	ExtendedRtr
} MTR_Uart_CanMessageType_t;

typedef struct
{
	uint32_t ID;
	MTR_Uart_CanMessageType_t type;
	uint8_t length;
} MTR_Uart_CanRxMsgHeader_t;

typedef struct
{
	MTR_Uart_CanRxMsgHeader_t Rx_Header;        /*!< Contains message header. */
	uint32_t Data[2];                    		/*!< Contains received data. */
} MTR_Uart_CanRxMsg_t;

typedef struct
{
    void (*switchToRecieve) (AvaliableConnectionType_t);
#ifdef CONFIG_UART_SERIAL_CAN_ENABLE
    void (*userCanRecieve) (AvaliableConnectionType_t, MTR_Uart_CanRxMsg_t);
#endif
    void (*userReceive)(AvaliableConnectionType_t);
    void (*userTimeout1)(AvaliableConnectionType_t);
    void (*userTimeout2)(AvaliableConnectionType_t);
    void (*userTimeout3)(AvaliableConnectionType_t);
    void (*userTimeout4)(AvaliableConnectionType_t);

    uint8_t lastSendedByte;
    uint32_t totalBytesRecieved;
    uint32_t totalBytesSended;

    int8_t currentReceiveBufferNumber;

    MTR_Uart_BufferStruct_t* currentReceiveBuffer;

    MTR_Uart_BufferStruct_t rxBuffer[2];
    MTR_Uart_TxBufferStruct_t txBuffer;

    uint16_t numsOfRepeats[UART_TIMER_NUMS_TOTAL_AMOUNT];
    										//!< Timer num for do nothing
   											//!< Timer num for waiting 3.5 symbols (14 bites)
											//!< Timer num for waiting answer from modbus slave device, ~50 ms
    uint16_t timerCounterDown;
    uint8_t needTimerNum;
    MTR_Port_Direction_t direction;

    MTR_Uart_BufferStruct_t* callbacksReceiveBuffer;
    uint32_t callCallbacksMask;
    volatile bool_t blockReceive;

} Uart_CommunicationStruct_t;

typedef struct
{
    MDR_UART_TypeDef* origin;   // MDR_UART
    uint32_t rst_clk;       	    // Clock
    uint32_t baud;      	        // Speed
    uint8_t dataLength;      	    // Datalenght
    uint8_t stopBites;				//
    uint8_t parity;					//
    uint8_t interrupt;        	    // Interrupt
    MTR_Uart_Type_t type;		    //Type RS
    bool_t enableEnablePinInvesion;
    MTR_Port_SomePin_t enPin;       //Enable pin
    MTR_Port_SomePin_t txPin;       // txPin-Pin
    MTR_Port_SomePin_t rxPin;       // rxPin-Pin
    bool_t inited;

} Uart_SettingsStruct_t;

/**
  * @brief  Initializes the UARTx peripheral according to the specified
  *         parameters in the UART struct. Timer defined by CONFIG_UART_RS485_TIMER.
  * @param  uart: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART1, UART2.
  * @retval Nothing.
  */
void MTR_Uart_init(AvaliableConnectionType_t uart);

void MTR_Uart_initGPIO(AvaliableConnectionType_t uart);

/**
 * @brief Deinit UART function. After this, u can change uart parameters and init again it with new parameters;
 * @param uart The name of uart, can be UART1 or UART2;
 * @return Nothing
 *
 */
void MTR_Uart_deInit(AvaliableConnectionType_t uart);


/**
 * @brief Start UART function
 * @param uart The name of uart, can be UART1 or UART2;
 * @return Nothing
 *
 */
void MTR_Uart_start(AvaliableConnectionType_t uart);


/**
 * @brief Stop uart function
 * @param uart : The name of uart, can be UART1 or UART2;
 * @return Nothing
 *
 */
void MTR_Uart_stop(AvaliableConnectionType_t uart);


/**
 * @brief Returns UART control structure
 * @param uart: The name of uart, can be UART1 or UART2.
 * @return pointer to  the contol struct;
 *
 */
Uart_SettingsStruct_t* MTR_Uart_getSettingsStruct(AvaliableConnectionType_t uart);


/**
 * @brief Get UART type (RS232 or RS485)
 * @param uart: The name of uart, can be UART1 or UART2.
 * @return UART type;
 *
 */
MTR_Uart_Type_t MTR_Uart_getType(AvaliableConnectionType_t uart);


/**
 * @brief Set uart type.
 * @param uartName The name of uart, can be UART1 or UART2;
 * @param newType new uart type, can be RS232 or RS485
 * @return Nothing
 */
void MTR_Uart_setType(AvaliableConnectionType_t uart, MTR_Uart_Type_t newType);


/**
 * @brief Get UART speed (bods 115200 for example)
 * @param uart: The name of uart, can be UART1 or UART2.
 * @return bods speed;
 *
 */
uint32_t MTR_Uart_getSpeed(AvaliableConnectionType_t uart);


/**
 * @brief Set UART speed (bods 115200 for example)
 * @param uart The name of uart, can be UART1 or UART2.
 * @param baud baud speed
 * @return Nothing;
 *
 */
void MTR_Uart_setSpeed(AvaliableConnectionType_t uart, uint32_t baud);


/**
 * @brief Get UART data length
 * @param uart The name of uart, can be UART1 or UART2.
 * @return size of data length (5..8 byte);
 *
 */
uint8_t MTR_Uart_getDataLength(AvaliableConnectionType_t uart);


/**
 * @brief Set UART data length
 * @param uart The name of uart, can be UART1 or UART2.
 * @param dataLength size of data length (5..8 byte);
 * @return void
 */
void MTR_Uart_setDataLength(AvaliableConnectionType_t uart, uint8_t dataLength);


/**
 * @brief Set UART stop bit size
 * @param uart The name of uart, can be UART1 or UART2.
 * @return amount of stop bites (1 or 2).
 */
uint8_t MTR_Uart_getStopBites(AvaliableConnectionType_t uart);


/**
 * @brief Set UART stop bit size
 * @param uart The name of uart, can be UART1 or UART2.
 * @param stopBites amount of stop bites (1 or 2).
 * @return void
 */
void MTR_Uart_setStopBites(AvaliableConnectionType_t uart, uint8_t stopBites);


/**
 * @brief Set UART parity
 * @param uart The name of uart, can be UART1 or UART2.
 * @return 0 if no parity, 1 if odd, 2 if even
 */
uint8_t MTR_Uart_getParity(AvaliableConnectionType_t uart);


/**
 * @brief Set UART parity
 * @param uart The name of uart, can be UART1 or UART2.
 * @param parity 0 if no parity, 1 if odd, 2 if even
 * @return void
 */
void MTR_Uart_setParity(AvaliableConnectionType_t uart, uint8_t parity);


/*
 * @brief Read byte from uart
 * @param uart: The name of uart, can be UART1 or UART2.
 * @return byte:  byte
 *
 */
uint8_t MTR_Uart_readByte(AvaliableConnectionType_t uart);


/*
 * @brief Add byte to uart transmit buffer
 * @param uart: The name of uart, can be UART1 or UART2.
 * @param byte: byte
 * @return void
 *
 */
void MTR_Uart_addByte(AvaliableConnectionType_t uart, uint8_t byte);


/*
 * @brief Add half word to uart buffer
 * @param uart: The name of uart, can be UART1 or UART2.
 * @param byte: half word
 * @return void
 *
 */
void MTR_Uart_addHalfWord(AvaliableConnectionType_t uart, uint16_t halfWord);


/*
 * @brief Add word to uart buffer
 * @param uart: The name of uart, can be UART1 or UART2.
 * @param byte: word
 * @return void
 *
 */
void MTR_Uart_addWord(AvaliableConnectionType_t uart, uint32_t word);


/*
 * @brief Add string to uart buffer
 * @param uart: The name of uart, can be UART1 or UART2.
 * @param pointer: pointer to string
 * @param end_cmd: symbol will be send in addition after data.
 *                  Can be NONE, LFCR, LF, CR, CRLF, COLON, SP, DOT
 * @return E_NULL_POINTER if pointer is null
 *
 */
errorType MTR_Uart_addString(AvaliableConnectionType_t uart, char* ptr, MTR_Uart_Lastbyte_t end_cmd);


/*
 * @brief Add byte array to uart buffer
 * @param uart: The name of uart, can be UART1 or UART2.
 * @param data: pointer to data
 * @param cnt : count of bytes need to be send
 * @return E_NULL_POINTER if pointer is null, or cnt = 0
 */
errorType MTR_Uart_addArray(AvaliableConnectionType_t uart, uint8_t const* data, uint16_t cnt);


/**
 * @brief Add CRC-16 to buffer, using specified function
 * CRC code calc based on current bytes in buffer.
 * @param uart uart name
 * @param crc16func CRC-16 function name;
 * @return nothing
 *
 */
void MTR_Uart_addCrc16(AvaliableConnectionType_t uart, uint16_t(*crc16Func)(uint8_t*, uint32_t));


/**
 * @brief Reset current buffer rx pointer. Reset read, write nums and data pointer
 * @param buffer : Pointer to valid buffer
 * @return Nothing
 *
 */
void MTR_Uart_resetRxPointer(AvaliableConnectionType_t uart, MTR_Uart_BufferStruct_t* buffer);
void MTR_Uart_resetBuffer(MTR_Uart_BufferStruct_t* buffer);


/**
 * @brief Reset current buffer tx pointer. Reset read, write nums and data pointer
 * @param uart : The name of uart, can be UART1 or UART2;
 * @return Nothing
 */
void MTR_Uart_resetTxPointer(AvaliableConnectionType_t uart);


/**
 * @brief Update RS-485 controller direction.
 * @param uart : The name of uart, can be UART1 or UART2;
 * @param direction: direction. Can be IN or OUT;
 * @return Nothing
 */
void MTR_Uart_setDirection(AvaliableConnectionType_t uart, MTR_Port_Direction_t direction);


/**
 * @brief return need current Rs485Direction from main structure.
 * @param uart uart name
 * @return IN or OUT driver status
 *
 */

MTR_Port_Direction_t MTR_Uart_getDirection(AvaliableConnectionType_t uart);

/**
 * @brief Start transmit added message
 * @param uart : The name of uart, can be UART1 or UART2;
 * @return E_OUT_OF_MEMORY if buffer is full, E_BUSY if UART is recieving now, else E_NO_ERROR
 *
 */
errorType MTR_Uart_initTransmite(AvaliableConnectionType_t uart);


/**
 * @brief Disable interrupt and send next byte. Enable interrupts. This func runs from interrupt
 * @param uart : The name of uart, can be UART1 or UART2;
 * @return void
 *
 */
void MTR_Uart_sendNextByte(AvaliableConnectionType_t uart);


/**
 * @brief Enable interrupts from specified event
 * @param uart : The name of uart, can be UART1 or UART2;
 * @param uart_it: specifies the interrupt pending bit to clear.
  *         This parameter can be one of the following values:
  *           @arg UART_IT_OE:  Buffer overflow interrupt (UARTOEINTR).
  *           @arg UART_IT_BE:  Line break interrupt (UARTBEINTR).
  *           @arg UART_IT_PE:  Parity error interrupt (UARTPEINTR).
  *           @arg UART_IT_FE:  Frame structure error interrupt (UARTFEINTR).
  *           @arg UART_IT_RT:  Data input timeout interrupt (UARTRTINTR).
  *           @arg UART_IT_TX:  Transmitter interrupt (UARTTXINTR).
  *           @arg UART_IT_RX:  Receiver interrupt (UARTRXINTR).
  *           @arg UART_IT_DSR: Line nUARTDSR change interrupt (UARTDSRINTR).
  *           @arg UART_IT_DCD: Line nUARTDCD change interrupt (UARTDCDINTR).
  *           @arg UART_IT_CTS: Line nUARTCTS change interrupt (UARTCTSINTR).
  *           @arg UART_IT_RI:  Line nUARTRI change interrupt (UARTRIINTR).
 * @return Nothing
 *
 */
void MTR_Uart_enableInterrupt(AvaliableConnectionType_t uart, uint32_t uart_it);


/**
  * @brief  Clears the UARTx’s interrupt pending bits.
  * @param  UARTx: Select the UART or the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART1, UART2.
  * @param uart_it: specifies the interrupt pending bit to clear.
  *         This parameter can be one of the following values:
  *           @arg UART_IT_OE:  Buffer overflow interrupt (UARTOEINTR).
  *           @arg UART_IT_BE:  Line break interrupt (UARTBEINTR).
  *           @arg UART_IT_PE:  Parity error interrupt (UARTPEINTR).
  *           @arg UART_IT_FE:  Frame structure error interrupt (UARTFEINTR).
  *           @arg UART_IT_RT:  Data input timeout interrupt (UARTRTINTR).
  *           @arg UART_IT_TX:  Transmitter interrupt (UARTTXINTR).
  *           @arg UART_IT_RX:  Receiver interrupt (UARTRXINTR).
  *           @arg UART_IT_DSR: Line nUARTDSR change interrupt (UARTDSRINTR).
  *           @arg UART_IT_DCD: Line nUARTDCD change interrupt (UARTDCDINTR).
  *           @arg UART_IT_CTS: Line nUARTCTS change interrupt (UARTCTSINTR).
  *           @arg UART_IT_RI:  Line nUARTRI change interrupt (UARTRIINTR).
  *
  * @return None
  */
void MTR_Uart_clearITbit(AvaliableConnectionType_t uart, uint32_t uart_it);


/**
 * @brief Enable interrupts from TX and RX, and overflow event
 * @param uart : The name of uart, can be UART1 or UART2;
 * @retirn Nothing
 */
void MTR_Uart_enableInterrupts(AvaliableConnectionType_t uart);


/**
 * @brief Enable interrupts in NVIC contoller
 * @param uart : The name of uart, can be UART1 or UART2;
 * @return Void
 */
void MTR_Uart_initNvic(AvaliableConnectionType_t uart);


/**
 * @brief Update tx pin parameters
 * @param uartName  The name of uart, can be UART1 or UART2;
 * @param port tx port to initialize
 * @param pin tx pin to initialize
 * @param func function of pin (ALTER, PORT, OVERRID, MAIN)
 * @return Nothing
 */
void MTR_Uart_setTxPin(AvaliableConnectionType_t uart, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func);


/**
 * @brief Update rx pin parameters
 * @param uartName  The name of uart, can be UART1 or UART2;
 * @param port rx port to initialize
 * @param pin rx pin to initialize
 * @param func function of pin (ALTER, PORT, OVERRID, MAIN)
 * @return Nothing
 *
 */
void MTR_Uart_setRxPin(AvaliableConnectionType_t uart, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Func_t func);


/**
 * @brief Update en pin parameters. This pin use for RS485
 * @param uartName  The name of uart, can be UART1 or UART2;
 * @param port tx port to initialize
 * @param pin tx pin to initialize
 * @param inversion enable inversion for DE/nRE pin. (Usual High levev for Tx, Low for Rx. With this bit High levev for Rx, Low for Tx)
 * @return Nothing
 *
 */
void MTR_Uart_setEnPin(AvaliableConnectionType_t uart, MTR_Port_Name_t port, MTR_Port_Pin_t pin, bool_t inversion);


/**
 *	@brief Get template parameters in ctructure.
 *	Use this function for init UART structure in RAM after reset.
 *	@param uart  uart name
 *	@return Pointer to structure with parameters
 */
Uart_CommunicationStruct_t* MTR_Uart_getCommunicationStruct(AvaliableConnectionType_t uart);


/**
 * @brief Set the timer of repeats need to some time. 1 count = time of transmit 4 bites with 115200 baud.
 * [0] is delay when need to set rs485 to IN after OUT
 * [1] is time between tx messages
 * [2] is time between rx messages
 * [3] used by Modbus. This is 50 ms delay
 * @param uart The name of uart, can be UART1 or UART2;
 * @param repeatNum repeat number of numsOfRepeats (0...3)
 * @param value number of repeats (0 - 0xFFFF)
 * @return void
 */
void MTR_Uart_setNumOfRepeats(AvaliableConnectionType_t uart, uint16_t repeatNum, uint16_t value);


/**
 * @brief Stop the timer and update counter to need repeat number.
 * @param uart uart name
 * @param timerNum repeat number of numsOfRepeats
 * @return E_INVALID_VALUE if number is no exist, else E_NO_ERROR
 */
errorType MTR_Uart_timerReset(AvaliableConnectionType_t uart, uint16_t timerNum);


/**
 * @brief Stop the timer and update counter to need repeat number, after it start the timer
 * @param uart uart name
 * @param timerNum repeat number of numsOfRepeats
 * @return E_INVALID_VALUE if number is no exist, else E_NO_ERROR
 */
void MTR_Uart_timerRefresh(AvaliableConnectionType_t uart, uint16_t timerNum);


/**
 * @brief This func runs from Timer interrupt. Counting inner counters. If need counter reached, switch driver to IN status.
 * There is 2 block defines. BLOCK_WHEN_UART_TIMER_SWITCH_DRIVER_TO_IN and BLOCK_WHEN_UART_TIMER_TIMEOUT
 * @param uart uart name
 * @return nothing
 */
void MTR_Uart_updateTimerNumsFromIRQ(AvaliableConnectionType_t uart);


/**
 * @brief Clear flags.
 * Runs from interrupt
 * @param uart uart name
 * @return void
 */
void MTR_Uart_clearOtherFlags(AvaliableConnectionType_t uart);


/**
 * @brief Function for use in interrupt, recieve and put byte to buffer
 * @param uart uart name
 * @return void
 */
void MTR_Uart_rxInterruptHandler(AvaliableConnectionType_t uart, uint8_t byte);


/**
 * @Brief Function return amount of bytes for transmit;
 * @param uart uart name
 * @return size of bytes
 */
uint16_t MTR_Uart_getTxBufferAmount(AvaliableConnectionType_t uart);

#ifdef CONFIG_UART_SERIAL_CAN_ENABLE
/**
 * @Brief Function return amount of bytes for transmit;
 * @param uart uart name
 * @param buffer recieve uart buffer
 * @param uart uart identifier
 * @return size of bytes
 */
bool_t MTR_Uart_parseToCanMessage(MTR_Uart_CanRxMsg_t* messageTo, MTR_Uart_BufferStruct_t* buffer, AvaliableConnectionType_t uart);

void MTR_Uart_addCanMessage(AvaliableConnectionType_t uart, MTR_Uart_CanRxMsg_t messageFrom);

#endif

uint32_t MTR_Uart_getTimerBaudSpeed();

//need to custom call if defined CONFIG_UART_SELFCALL_CALLBACKS
void MTR_Uart_callCallbacks();


#ifdef __cplusplus
}
#endif

#endif /* MTR_UART_BASE_H_ */

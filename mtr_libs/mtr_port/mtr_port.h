/**
 * @file
 * @brief This file contain function for port operating.
 *
 * You need to do next steps : deInit -> init -> set/reset
 * @ingroup g_MilandrGit__1986VE92__Submodules__Periphery__Gpio__mtr_port
 *
 *
 */
#ifndef MTR_PORT_H_
#define MTR_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "mtr_service/mtr_service.h"
#include "MDR32F9Qx_port.h"

#ifndef CONFIG_PORT_CONNECTED_PIN_AMOUNT
#define CONFIG_PORT_CONNECTED_PIN_AMOUNT 8
#endif

typedef enum{

    PORTA = 0,
    PORTB = 1,
    PORTC = 2,
    PORTD = 3,
    PORTE = 4,
    PORTF = 5

}MTR_Port_Name_t;


typedef struct{

	MTR_Port_Name_t portName;
	MDR_PORT_TypeDef* portOriginName;
	uint32_t RST_CLK_PCLK;

}MTR_Port_t;


/*
 * Port modes
 */


typedef enum{

    ANALOG = PORT_MODE_ANALOG,
	DIGITAL = PORT_MODE_DIGITAL

}MTR_Port_Mode_t;


typedef enum{

	PIN0 = 0x0001U,  /*!< Pin 0 selected */
	PIN1 = 0x0002U,  /*!< Pin 1 selected */
	PIN2 = 0x0004U,  /*!< Pin 2 selected */
	PIN3 = 0x0008U,  /*!< Pin 3 selected */
	PIN4 = 0x0010U,  /*!< Pin 4 selected */
	PIN5 = 0x0020U,  /*!< Pin 5 selected */
	PIN6 = 0x0040U,  /*!< Pin 6 selected */
	PIN7 = 0x0080U,  /*!< Pin 7 selected */
	PIN8 = 0x0100U,  /*!< Pin 8 selected */
	PIN9 = 0x0200U,  /*!< Pin 9 selected */
	PIN10 = 0x0400U,  /*!< Pin 10 selected */
	PIN11 = 0x0800U,  /*!< Pin 11 selected */
	PIN12 = 0x1000U,  /*!< Pin 12 selected */
	PIN13 = 0x2000U,  /*!< Pin 13 selected */
	PIN14 = 0x4000U,  /*!< Pin 14 selected */
	PIN15 = 0x8000U,  /*!< Pin 15 selected */
	PINAll = 0xFFFFU  /*!< All pins selected */

}MTR_Port_Pin_t;


typedef enum
{
  IN            = 0x0,
  OUT           = 0x1
}MTR_Port_Direction_t;


typedef enum
{
  PORT        = 0x0,
  MAIN        = 0x1,
  ALTER       = 0x2,
  OVERRID     = 0x3

}MTR_Port_Func_t;


typedef enum
{
  OFF = PORT_OUTPUT_OFF,
  SLOW = PORT_SPEED_SLOW,
  NORMAL = PORT_SPEED_FAST,
  MAX = PORT_SPEED_MAXFAST,

}MTR_Port_Speed_t;


typedef struct{

    MTR_Port_Name_t portName;
    MTR_Port_Pin_t portPin;
    MTR_Port_Mode_t mode;
    MTR_Port_Func_t func;
    MTR_Port_Direction_t direction;
    MTR_Port_Speed_t speed;
    status_t pullUp;
    status_t pullDown;

}MTR_Port_SomePin_t;


typedef struct{

	MTR_Port_Name_t portName;
	MTR_Port_Pin_t portPin;
	MTR_Port_Direction_t direction;
	bool_t* connectedWith;

}MTR_Port_ConnectedPin_t;


typedef struct{
	MTR_Port_ConnectedPin_t pins[CONFIG_PORT_CONNECTED_PIN_AMOUNT];
	uint8_t currentPinAmount;
}MTR_Port_ConnectedPins_t;


/**
  * @brief  Deinitializes the port.
  * @param  port port to initializing. Can be PORTA, PORTB ... PORT15, PINALL or combination of ports.
  * @return None
  */

void MTR_Port_deInit(MTR_Port_Name_t port);


/**
  * @brief  Initializes the port peripheral according to the specified parameters.
  * @param  port port to initializing. Can be PORTA, PORTB ... PORT15, PINALL or combination of ports.
  * @param  pin pin to initializing. Can be PIN0, PIN1.. PIN16 or compination of pins.
  * @param  mode port mode. Can be ANALOG or DIGITAL.
  * @param  func port function.
  * @param  direction : can be IN, OUT.
  * @param  pullUp can be enable or disable
  * @param  pullDown can be enable or disable
  * @return None
  */
void MTR_Port_init(MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Mode_t mode, MTR_Port_Func_t func, MTR_Port_Direction_t direction, MTR_Port_Speed_t speed, status_t pullUp, status_t pullDown);

void MTR_Port_initAnalog(MTR_Port_Name_t port, MTR_Port_Pin_t pin);

void MTR_Port_initDigital(MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Direction_t direction);



/**
  * @brief  Initializes the port peripheral according to the specified parameters.
  * @param  initUartSruct MTR_Port_SomePin_t structure to init
  * @return None
  */

void MTR_Port_initWithStruct(MTR_Port_SomePin_t initUartSruct);


/**
  * @brief  Sets the selected data port bits.
  * @param  port: specifies the port name (PORTA, PORTB...)
  * @param  pin:  specifies the port bits to be written.
  *         This parameter can be any combination of MTR_PortPins_t (0..15).
  * @return None
  */
void MTR_Port_setPin(MTR_Port_Name_t port, MTR_Port_Pin_t pin);




/**
  * @brief  Resets the selected data port bits.
  * @param  port: specifies the port name (PORTA, PORTB...)
  * @param  pin:  specifies the port bits to be written.
  *         This parameter can be any combination of MTR_PortPins_t (0..15).
  * @return None
  */
void MTR_Port_resetPin(MTR_Port_Name_t port, MTR_Port_Pin_t pin);


/**
  * @brief  Sets the selected data port bits.
  * @param  port: specifies the port name (PORTA, PORTB...)
  * @param  pin:  specifies the port bits to be written.
  *         This parameter can be any combination of MTR_Port_Pin_t (0..15).
  * @return Current bit status
  */
bool_t MTR_Port_getBit(MTR_Port_Name_t port, MTR_Port_Pin_t pin);


/**
  * @brief  Reads the all bits of specified port .
  * @param  port: where x can be (A..F) to select the PORT peripheral.
  * @return PORT input data port value.
  */
uint16_t MTR_Port_getPort(MTR_Port_Name_t port);


/**
  * @brief  Write the specified port bit with need status.
  * @param  port where x can be (A..F) to select the PORT peripheral.
  * @param  pin  pin to write status.
  * @param  status need bit status
  * @return PORT input data port value. Bits (16..31) are always 0.
  */
void MTR_Port_writeBit(MTR_Port_Name_t port, MTR_Port_Pin_t pin, bitStatus_t status);


/**
  * @brief  Write the specified port bit with need status.
  * @param  port where x can be (A..F) to select the PORT peripheral.
  * @return Pointer to MDR_PORT_TypeDef struct using port
  */
MDR_PORT_TypeDef* MTR_Port_getOriginStruct(MTR_Port_Name_t port);


void MTR_Port_connectPin(MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Port_Direction_t direction, bool_t* value);

void MTR_Port_updatePins();

void MTR_Port_getPinFromUint32(uint32_t value, MTR_Port_SomePin_t* pinToWrite);

uint32_t MTR_Port_getUint32FromPin(MTR_Port_SomePin_t* pin);

void __attribute__((weak)) MTR_Port_pinStateChanged(MTR_Port_ConnectedPin_t pin);
void __attribute__((weak)) MTR_Port_incomePinStatesDataChanged();

#ifdef __cplusplus
}
#endif

#endif /* MTR_PORT_H_ */

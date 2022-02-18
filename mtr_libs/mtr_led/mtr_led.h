/**
 * @file
 * @brief This file contain function for work with GPIO led.
 *
 * This is full description of the file
 * @ingroup g_MilandrGit__1986VE92__Submodules__Periphery__Gpio__mtr_led
 *
 */

#ifndef MTR_LED_H
#define MTR_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "mtr_port/mtr_port.h"

#ifndef CONFIG_LED_MAX_AMOUNT
#define CONFIG_LED_MAX_AMOUNT 8
#endif

/**
 * @brief Specify using leds name.
 * This name using in all led functions, all leds specified
 */
typedef enum {
    LED0 = 0,   ///< Led 0
    LED1 = 1,   ///< Led 1
    LED2 = 2,   ///< Led 2
    LED3 = 3,   ///< Led 3
    LED4 = 4,   ///< Led 4
    LED5 = 5,   ///< Led 5
    LED6 = 6,   ///< Led 6
    LED7 = 7,   ///< Led 7
    LED8 = 8,   ///< Led 8
    LED9 = 9,   ///< Led 9
}MTR_Led_Name_t;


/**
 * @brief Specify using leds status.
 *
 * This enum use for specify led status
 */
typedef enum {
    LED_OFF = 0,  ///< Led off
    LED_ON        ///< Led on
}MTR_Led_Status_t;


/***** Structure of LED *****/
typedef struct {
	MTR_Port_Name_t       ledPort;    //Port
	MTR_Port_Pin_t        ledPin;     //Pin
	MTR_Led_Status_t      ledStatus;  //Init
	bool_t 				inited;
}MTR_Led_t;


/*
 * @brief Led initializing. Enable clock and set off status of pin.
 * @param led : led name. Can be LED0...LED9
 * @param portName : port of led. Can be PORTA..PORTF
 * @param portPin : pin of led. Can be PIN0.. PIN15
 *
 * @return return E_OUT_OF_RANGE if led field is not filled, E_NO_ERROR if everything is okay
 */
errorType MTR_Led_init(MTR_Led_Name_t led, MTR_Port_Name_t portName, MTR_Port_Pin_t portPin);


/*
 * @brief Switch led status to on
 * @param led : led name. Can be LED0...LED9
 * @return Nothing
 */
void MTR_Led_on(MTR_Led_Name_t led);


/*
 * @brief Switch led status to off
 * @param led : led name. Can be LED0...LED9
 * @return Nothing
 */
void MTR_Led_off(MTR_Led_Name_t led);


/*
 * @brief Switch led status to another.
 * @param led : led name. Can be LED0...LED9
 * @return Nothing
 */
void MTR_Led_switch(MTR_Led_Name_t led);


/*
 * @brief Return current led status from inner struct.
 * @param led : led name. Can be LED0...LED9
 * @return 0 if led off, 1 if led on
 */
uint8_t MTR_Led_getStatus(MTR_Led_Name_t led);


#ifdef __cplusplus
}
#endif


#endif /* MTR_LED_H_ */

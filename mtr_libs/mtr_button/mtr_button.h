/**
 * @file
 * @brief This file contain function for work with GPIO buttons.
 *
 * This is full description of the file
 * @ingroup g_MilandrGit__1986VE92__Submodules__Periphery__Gpio__mtr_button
 *
 *
 */

#ifndef MTR_BUTTON_H
#define MTR_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mtr_port/mtr_port.h"
#include "mtr_service/mtr_service.h"
#include "config.h"


/**
 * @brief Specify using buttons name.
 *
 * This name using in all buttons functions, all buttons specified
 */
typedef enum
{
  BTN0,         ///< Zero num button
  BTN1,         ///< First num button
  BTN2,         ///< Second num button
  BTN3,         ///< Third num button
  BTN4,         ///< Fourth num button
  BTN5,         ///< Fifth num button
  BTN6,         ///< Sixth num button
  BTN7,         ///< Seventh num buttom
  BTN8,         ///< Eighth num button
  BTN9          ///< Eighth num button
}MTR_Button_Name_t;

typedef enum{
	NormalButton,
	ReverseButton
}MTR_Button_Type_t;


/*
 * Button struct
 */
typedef struct {
  MTR_Button_Name_t 	buttonName;  			// Name
  MTR_Port_Name_t    	buttonPort; 			// Port
  MTR_Port_Pin_t		buttonPin;	      		// Pin
  MTR_Button_Type_t 	type;
  bool_t 				buttonStatus;
  bool_t				butonHolding;
  int 				    buttonValue;
  int					holdingValue;
}MTR_Button_t;


/**
 * @brief Button initializing function.
 * @param button button MTR_Button_Name_t name.
 * @param port Port of button
 * @param pin Pin of button
 * @retval  E_INVALID_VALUE if button amount equals maximum limits. In others case return E_NO_ERROR
 */
errorType MTR_Button_init(MTR_Button_Name_t button, MTR_Port_Name_t port, MTR_Port_Pin_t pin, MTR_Button_Type_t type);


/**
 * @brief Function returns last update button status.
 * @param button  Button to read status. All buttons have individual MTR_Button_Name_t name.
 * @retval pointer to bool button value
*/
bool_t const * MTR_Button_read(MTR_Button_Name_t button);


/**
 * @brief Update current button status. Use it periodically
 * @retval Nothing
 *
 */
void MTR_Button_update();

void __attribute__((weak)) MTR_Button_buttonStates(uint32_t code);
void __attribute__((weak)) MTR_Button_buttonPressed(uint32_t code);
void __attribute__((weak)) MTR_Button_buttonReleased(uint32_t code);
void __attribute__((weak)) MTR_Button_buttonHolded(uint32_t code);

#ifdef __cplusplus
}
#endif

#endif /* MTR_BUTTON_H */

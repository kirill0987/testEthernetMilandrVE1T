#ifndef MTR_TIMER_H
#define MTR_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_timer.h"
#include "config.h"
#include "mtr_service/mtr_service.h"

#ifndef CONFIG_CLOCK_TIMER_DIVISOR
    #warning "Need to define prescaller. Default it is TIMER_HCLKdiv8"
#endif

#ifndef CONFIG_CLOCK_TIMER_MHZ
    #warning "Need to define timer frequency. Default it is (CONFIG_CURRENT_CLOCK_MHZ / (1 << CONFIG_CLOCK_TIMER_DIVISOR)"
#endif

#ifndef CONFIG_TIMER_NVIC_PRIORITY
    #warning "Need to define timer interrupt priority."
#endif


typedef enum
{
	TIMER1 = 0,
	TIMER2 = 1,
	TIMER3 = 2
}MTR_Timer_Name_t;


//--------------------------------------------------------------
// Structures
//--------------------------------------------------------------
typedef struct {
	MTR_Timer_Name_t 		 	timerName;         	// Name
	MDR_TIMER_TypeDef*   		timerOrigin;       	// Milandr Timer Structure
	IRQn_Type					timerIRQ;  			// Milandr Interrupt structure
	uint32_t             	    rst_clk_pclk;		// Milandr rst clk
}MTR_Timer_t;


/**
  * @brief  Initializes the timer with parameters.  Timer counts from value to period with UP direction
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param	value : start value
  * @param	period : end value of count.
  * @param	prescaller : timer prescaller
  * @return None
  */
void MTR_Timer_init(MTR_Timer_Name_t timer, uint16_t value, uint16_t period, uint16_t prescaller);


/**
  * @brief  Initializes the timer with structure.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param	timerStruct : pointer to init structure of TIMER_CntInitTypeDef type
  * @return None
  */
void MTR_Timer_initAlt(MTR_Timer_Name_t timer, TIMER_CntInitTypeDef* timerStruct);


/**
  * @brief  Deinitializes the timer peripheral registers to their default reset values.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return None
  */
void MTR_Timer_deInit(MTR_Timer_Name_t timer);


/**
  * @brief  Initializes the timer with structure.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param	frequency : need frequency
  * @return None
  */
void MTR_Timer_initFreq(MTR_Timer_Name_t timer, uint16_t frequency);


/**
  * @brief  Update timer counter to zero and stop the timer.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return None
  */
void MTR_Timer_reset(MTR_Timer_Name_t timer);


/**
  * @brief  Enable controller NCIV interrupt.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return None
  */
void MTR_Timer_initNvic(MTR_Timer_Name_t timer);


/* @brief Config periph interrupt.
 * @param timer: timer to operate. Can be TIMER1.. TIMER3.
 * @param timer_it: specifies the TIMER interrupts sources to be enabled or disabled.
 *         This parameter can be any combination of the following values:
 *           @arg TIMER_STATUS_CNT_ZERO:         the (CNT == 0) condition occured;
 *           @arg TIMER_STATUS_CNT_ARR:          the (CNT == ARR) condition occured;
 *           @arg TIMER_STATUS_ETR_RISING_EDGE:  the ETR rising edge occured;
 *           @arg TIMER_STATUS_ETR_FALLING_EDGE: the ETR falling edge occured;
 *           @arg TIMER_STATUS_BRK:              the (BRK == 1) condition occured;
 * @status : Need status of interrupt. Can be true or false.
 * @return Nothing
 */
void MTR_Timer_initIT(MTR_Timer_Name_t timer, uint32_t timer_it, status_t status);


/**
  * @brief  Stop timer counting.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return None
  */
void MTR_Timer_stop(MTR_Timer_Name_t timer);


/**
  * @brief  Start timer counting.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return None
  */
void MTR_Timer_start(MTR_Timer_Name_t timer);


/**
  * @brief  Checks whether the specified TIMERx Status flag is set or not.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param  flag: specifies the flag to check.
  *         This parameter can be one of the following values:
  *           @arg TIMER_STATUS_CNT_ZERO:         the (CNT == 0) condition occured;
  *           @arg TIMER_STATUS_CNT_ARR:          the (CNT == ARR) condition occured;
  * @return Current Status flag state (true or false).
  */
bool_t MTR_Timer_getFlagStatus(MTR_Timer_Name_t timer, uint32_t flag);


/**
  * @brief  Clears the TIMERx's pending flags.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param  flag: specifies the flag bit mask to clear.
  *         This parameter can be any combination of the following values:
  *           @arg TIMER_STATUS_CNT_ZERO:         the (CNT == 0) condition occured;
  *           @arg TIMER_STATUS_CNT_ARR:          the (CNT == ARR) condition occured;
  * @return None
  */
void MTR_Timer_clearFlag(MTR_Timer_Name_t timer, uint32_t flag);


/**
  * @brief  Update timer current count.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param cnt: num for update
  * @return None
  */
void MTR_Timer_setCounter(MTR_Timer_Name_t timer, uint16_t cnt);


/**
  * @brief Returns timer current count.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return current count in timer
  */
uint16_t MTR_Timer_getCounter(MTR_Timer_Name_t timer);


/**
  * @brief  Update timer current prescaler.
  * @param timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param prescaler: num for update
  * @return None
  */
void MTR_Timer_setPrescaler(MTR_Timer_Name_t timer, uint16_t prescaler);


/**
  * @brief  Configures the TIMERx Counter Event source.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param  EventSource: specifies the Event source.
  *         This parameter can be one of the following values:
  *           @arg TIMER_EvSrc_None: no events;
  *           @arg TIMER_EvSrc_TM1:  selects TIMER1 (CNT == ARR) event;
  *           @arg TIMER_EvSrc_TM2:  selects TIMER2 (CNT == ARR) event;
  *           @arg TIMER_EvSrc_TM3:  selects TIMER3 (CNT == ARR) event;
  *           @arg TIMER_EvSrc_ETR:  selects ETR event.
  * @return None
  */
void MTR_Timer_eventSourceConfig(MTR_Timer_Name_t timer, uint32_t eventSource);


/**
  * @brief  Configures the TIMERx count direction.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param  Direction: specifies the Timer count direction.
  *         This parameter can be one of the following values:
  *           @arg TIMER_CntDir_Up: increments the Timer TIMERx_CNT counter value;
  *           @arg TIMER_CntDir_Dn: decrements the Timer TIMERx_CNT counter value.
  * @return None
  */
void MTR_Timer_setCountDirection(MTR_Timer_Name_t timer, uint32_t direction);


/**
  * @brief  Returns pointer to origin MDR struct of timer.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return MDR_TIMER_TypeDef type pointer
  */
MDR_TIMER_TypeDef* MTR_Timer_getOriginStruct(MTR_Timer_Name_t timer);


/**
  * @brief  Set custom priority of timer.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @param  priority: need priority of timer
  * @return None
  */
void MTR_Timer_setPriority(MTR_Timer_Name_t timer, uint8_t priority);


/**
  * @brief  Get Irq of timer.
  * @param  timer: timer to operate. Can be TIMER1.. TIMER3.
  * @return IRQn_Type of timer
  */
IRQn_Type MTR_Timer_getIrq(MTR_Timer_Name_t timer);

#ifdef __cplusplus
}
#endif

#endif /* MTR_TIMER_H_ */

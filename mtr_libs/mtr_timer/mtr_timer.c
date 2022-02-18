#include "mtr_timer/mtr_timer.h"
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_rst_clk.h"

//--------------------------------------------------------------
// Definition of all Timers
// Position as Timer_NAME_t
//--------------------------------------------------------------
MTR_Timer_t TIMER[] = {
// Name,	Stuct,  	START CNT
  {TIMER1,	MDR_TIMER1, TIMER1_IRQn, RST_CLK_PCLK_TIMER1},
  {TIMER2,	MDR_TIMER2,	TIMER2_IRQn, RST_CLK_PCLK_TIMER2},
  {TIMER3,	MDR_TIMER3,	TIMER3_IRQn, RST_CLK_PCLK_TIMER3}
};

void MTR_Timer_init(MTR_Timer_Name_t timer, uint16_t value, uint16_t period, uint16_t prescaler)
{
	TIMER_CntInitTypeDef timerStruct;

    RST_CLK_PCLKcmd(TIMER[timer].rst_clk_pclk, ENABLE);
	TIMER_DeInit(TIMER[timer].timerOrigin);
    TIMER_BRGInit(TIMER[timer].timerOrigin, CONFIG_CLOCK_TIMER_DIVISOR);

    prescaler = (prescaler == 0) ? 1 : prescaler;

	timerStruct.TIMER_IniCounter 	     = value;
	timerStruct.TIMER_Prescaler 		 = prescaler - 1;  //0xffff - max
	timerStruct.TIMER_Period			 = period;		//0xffff- max
	timerStruct.TIMER_CounterDirection	 = TIMER_CntDir_Up;
	timerStruct.TIMER_CounterMode	   	 = TIMER_CntMode_ClkFixedDir;
	timerStruct.TIMER_EventSource    	 = TIMER_EvSrc_None;
	timerStruct.TIMER_FilterSampling  	 = TIMER_FDTS_TIMER_CLK_div_1;
	timerStruct.TIMER_ARR_UpdateMode  	 = TIMER_ARR_Update_Immediately;
	timerStruct.TIMER_ETR_FilterConf  	 = TIMER_Filter_1FF_at_TIMER_CLK;
	timerStruct.TIMER_ETR_Prescaler   	 = TIMER_ETR_Prescaler_None;
	timerStruct.TIMER_ETR_Polarity    	 = TIMER_ETRPolarity_NonInverted;
	timerStruct.TIMER_BRK_Polarity    	 = TIMER_BRKPolarity_NonInverted;

	TIMER_CntInit(TIMER[timer].timerOrigin, &timerStruct);
	//transmit structure to milandr lib
}

void MTR_Timer_initAlt(MTR_Timer_Name_t timer, TIMER_CntInitTypeDef* timerStruct)
{
    RST_CLK_PCLKcmd(TIMER[timer].rst_clk_pclk, ENABLE);
	TIMER_DeInit(TIMER[timer].timerOrigin);
	TIMER_BRGInit(TIMER[timer].timerOrigin, CONFIG_CLOCK_TIMER_DIVISOR);
	TIMER_CntInit(TIMER[timer].timerOrigin, timerStruct);
}

void MTR_Timer_deInit(MTR_Timer_Name_t timer)
{
	TIMER_DeInit(TIMER[timer].timerOrigin);
}

void MTR_Timer_initFreq(MTR_Timer_Name_t timer, uint16_t frequency)
{
	uint16_t prescaler;
	uint32_t period;
	uint32_t clk_frq = CONFIG_CLOCK_TIMER_HZ;

	if (frequency==0) {
		frequency=1;
	}

	if (frequency>clk_frq) { //check for limits
		frequency=clk_frq;
	}

	//block of selection of prescaler
	if (frequency < 122) {
		prescaler = 128;
	}
	else {
		prescaler = 1;
	}
	period = (clk_frq)/(frequency * prescaler);//calculate need value for timer

	MTR_Timer_init(timer, 0, period, prescaler);
}

void MTR_Timer_reset(MTR_Timer_Name_t timer)
{
	TIMER_SetCounter(TIMER[timer].timerOrigin, 0);
	MTR_Timer_stop(timer);
}

void MTR_Timer_initNvic(MTR_Timer_Name_t timer)
{
   // NVIC config
	NVIC_EnableIRQ(TIMER[timer].timerIRQ);
	NVIC_SetPriority(TIMER[timer].timerIRQ, CONFIG_TIMER_NVIC_PRIORITY);
}

void MTR_Timer_initIT(MTR_Timer_Name_t timer, uint32_t timer_it, status_t status)
{
	TIMER_ITConfig(TIMER[timer].timerOrigin, timer_it, status);
}

void MTR_Timer_stop(MTR_Timer_Name_t timer)
{
	TIMER_Cmd(TIMER[timer].timerOrigin, DISABLE);
}

void MTR_Timer_start(MTR_Timer_Name_t timer)
{
	TIMER_Cmd(TIMER[timer].timerOrigin, ENABLE);
}

bool_t MTR_Timer_getFlagStatus(MTR_Timer_Name_t timer, uint32_t flag)
{
	return TIMER_GetFlagStatus(TIMER[timer].timerOrigin, flag);
}

void MTR_Timer_clearFlag(MTR_Timer_Name_t timer, uint32_t flag)
{
	TIMER_ClearFlag(TIMER[timer].timerOrigin, flag);
}

void MTR_Timer_setCounter(MTR_Timer_Name_t timer, uint16_t cnt)
{
	TIMER_SetCounter(TIMER[timer].timerOrigin, cnt);
}

uint16_t MTR_Timer_getCounter(MTR_Timer_Name_t timer)
{
	return TIMER_GetCounter(TIMER[timer].timerOrigin);
}

void MTR_Timer_setPrescaler(MTR_Timer_Name_t timer, uint16_t prescaler)
{
	TIMER_SetCntPrescaler(TIMER[timer].timerOrigin, prescaler);
}

void MTR_Timer_eventSourceConfig(MTR_Timer_Name_t timer, uint32_t eventSource)
{
	TIMER_CntEventSourceConfig(TIMER[timer].timerOrigin, eventSource);
}

void MTR_Timer_setCountDirection(MTR_Timer_Name_t timer, uint32_t direction)
{
	TIMER_SetCounterDirection(TIMER[timer].timerOrigin, direction);
}

MDR_TIMER_TypeDef* MTR_Timer_getOriginStruct(MTR_Timer_Name_t timer)
{
	 return TIMER[timer].timerOrigin;
}

bool_t MTR_Timer_isStarted(MTR_Timer_Name_t timer)
{
	return TIMER[timer].timerOrigin->CNTRL & 0x1;
}

void MTR_Timer_setPriority(MTR_Timer_Name_t timer, uint8_t priority)
{
	NVIC_SetPriority(TIMER[timer].timerIRQ, priority);
}

IRQn_Type MTR_Timer_getIrq(MTR_Timer_Name_t timer)
{
	return TIMER[timer].timerIRQ;
}

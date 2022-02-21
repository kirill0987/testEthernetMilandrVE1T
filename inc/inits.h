#ifndef INC_INITS_H_
#define INC_INITS_H_

#include "inc/test.h"

#include "cmsis_boot/MDR1986VE1T.h"
#include "cmsis_lib/inc/MDR32F9Qx_eth.h"

void Clock_Init();

//	Обработка входящего фрейма и высылка ответа
void Ethernet_FillFrameTX(uint32_t frameL);
//	Заполнение массива FrameTx фреймом заданной длины
void ETH_TaskProcess(MDR_ETHERNET_TypeDef* ETHERNETx);

void initNetwork();

#endif /* INC_INITS_H_ */

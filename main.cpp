#include <stdio.h>
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_rst_clk.h"

#include "mtr_libs/mtr_service/mtr_service.h"
#include "mtr_libs/mtr_planner/mtr_planner.h"

#include "inc/inits.h"

int main(void)
{
  	disableInterrupts();

 	initDevice();
 	initData();
 	initGPIO();
 	initNetwork();
 	//initPlanner();

	__enable_irq();

	for (;;)
	{
		//MTR_Planner_runTasks();
	}
}

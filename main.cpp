#include <stdio.h>
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_rst_clk.h"


#include "inc/inits.h"
#include "inc/tasks.h"

int main(void)
{
	Clock_Init();
 	initNetwork();

	__enable_irq();

	for (;;)
	{
		 ETH_TaskProcess(MDR_ETHERNET1);
	}
}




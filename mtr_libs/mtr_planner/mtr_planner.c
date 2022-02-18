#include "mtr_planner.h"
#include "mtr_timer/mtr_timer.h"
#include <malloc.h>
#include <stdlib.h>

volatile Planner_t Planner;

static void MTR_Planner_updateForceRunTaskCounter();

uint64_t fixPeriod(uint64_t x, uint64_t period);
uint64_t findMinimalLeftTime();

#include <malloc.h>

void MTR_Planner_runTasks(){

	static MDR_TIMER_TypeDef* timerStruct = 0;
	uint64_t minVal = 0;

	if (!timerStruct){
		timerStruct = MTR_Timer_getOriginStruct(CONFIG_PLANNER_TIMER);
	}

	Task_t* currentTask = Planner.firstTask;
	Task_t* lastTask;

	if (Planner.timeAlreadyCounted != 0){

//		int32_t tcntStart = timerStruct->CNT;
//		timerStruct->CNT = 0;
//		timerStruct->ARR = 0xFFFF;

		for(;;)
		{
			if (currentTask->enabled && getBit(Planner.runTypeTask, currentTask->typeTask)){

				if (currentTask->timeLeft > Planner.timeAlreadyCounted) {
					currentTask->timeLeft -= Planner.timeAlreadyCounted;
				}
				else {
					currentTask->timeLeft = 0;
				}

				if (currentTask->timeLeft == 0){

#ifndef CONFIG_PLANNER_DISABLE_DISABLE_IRQ
					//CARE
					//DISABLE LAST
					//ENABLED AT END OF THIS FUNC
					__enable_irq();
					__disable_irq();
#endif
					currentTask->callback();
					lastTask = currentTask;

					//if not zero, task change self timeLeft
					if (currentTask->timeLeft == 0){
						currentTask->timeLeft = currentTask->timeout;
					}
				}
			}

			if (currentTask->nextTask) {
				currentTask = currentTask->nextTask;
			}
			else {
				break;
			}
		}

		Planner.timeAlreadyCounted = 0;

		//MTR_Timer_stop(TIMER2);

		minVal = findMinimalLeftTime();
		if (minVal > 0xFFFF){
			minVal = 0xFFFF;
		}

		//check already clicked ms from start this func
/*
		int32_t tcntEnd = (int32_t)timerStruct->CNT;

		if (tcntEnd > tcntStart){
			//usual
			minVal -= (tcntEnd - tcntStart);
		}
		else {
			//overfload already
			//do nothing
		}
*/

		//todo fix
		if (minVal == 0){
			minVal = 10;
		}

		//MTR_Timer_stop(TIMER2);

		timerStruct->ARR = minVal;

		//MTR_Timer_start(TIMER2);
		__enable_irq();
	}
}

void MTR_Planner_updateForceRunTaskCounter(){
	Planner.forceRunTaskCounter = CONFIG_PLANNER_FORCE_RUN_TASK_COUNTER;
}

const uint64_t targetFreq = 1000000;

void MTR_Planner_init(){

	MTR_Planner_addTask(MTR_Planner_updateForceRunTaskCounter, (CONFIG_PLANNER_FORCE_RUN_TASK_COUNTER / 10) ? (CONFIG_PLANNER_FORCE_RUN_TASK_COUNTER / 10) : 1, enable, UsualTask);
	MTR_Planner_updateForceRunTaskCounter();

	//1000 - just default value
	MTR_Timer_init(CONFIG_PLANNER_TIMER, 0, 100, (uint16_t) (CONFIG_CLOCK_TIMER_HZ / targetFreq));

	MTR_Timer_initIT(CONFIG_PLANNER_TIMER, TIMER_STATUS_CNT_ZERO, disable);
	MTR_Timer_initIT(CONFIG_PLANNER_TIMER, TIMER_STATUS_CNT_ARR, enable);
	MTR_Timer_initNvic(CONFIG_PLANNER_TIMER);
	MTR_Timer_setPriority(CONFIG_PLANNER_TIMER, 6);
	MTR_Timer_start(CONFIG_PLANNER_TIMER);
	Planner.runTypeTask = 0xff;
}

uint64_t fixPeriod(uint64_t x, uint64_t period){

	if (x > period){
		return x % period;
	}

	return x;
}

Task_t const * MTR_Planner_addTask(void (*function)(), uint64_t msTimeout, bool_t enabled, TypeTask_t taskType){
	return MTR_Planner_addMicroTask(function, msTimeout * 1000, enabled, taskType);
}

Task_t const * MTR_Planner_addMicroTask(void (*function)(), uint64_t microsTimeout, bool_t enabled, TypeTask_t taskType){
	//create new task
	Task_t* newTask = calloc(sizeof(Task_t), 1);
	uint64_t ranVal = (uint64_t) (rand() ^ rand());
	newTask->timeLeft = fixPeriod(ranVal, microsTimeout);
	newTask->timeout = microsTimeout;
	newTask->callback = function;
	newTask->nextTask = 0;
	newTask->enabled = enabled;
	newTask->typeTask = taskType;

	if(!Planner.firstTask)
	{
		//create first procedure
		Planner.firstTask = newTask;
	}
	else {
		Planner.lastTask->nextTask = newTask;
	}

	Planner.lastTask = newTask;

	return newTask;
}


Task_t const * MTR_Planner_getTask(void (*function)()){

	Task_t* task = Planner.firstTask;

	for (;;)
	{
		if (task->callback == function){
			return task;
		}

		if (task->nextTask){
			task = task->nextTask;
		}
		else {
			break;
		}
	}//1 for

	return 0;
}

void CONFIG_PLANNER_TIMER_IRQ(){

	static MDR_TIMER_TypeDef* timerStruct = 0;

	if (!timerStruct){
		timerStruct = MTR_Timer_getOriginStruct(CONFIG_PLANNER_TIMER);
	}

	timerStruct->STATUS &= ~ (TIMER_STATUS_CNT_ZERO | TIMER_STATUS_CNT_ARR);

	Planner.timeAlreadyCounted = timerStruct->ARR;
	uint64_t temp = Planner.timeAlreadyCounted;

	if (Planner.forceRunTaskCounter < 0) {
		MTR_Planner_runTasks();
		MTR_Planner_updateForceRunTaskCounter();
	}
	else {
		Planner.forceRunTaskCounter -= temp;
	}
}

uint64_t findMinimalLeftTime(){
	Task_t* task = Planner.firstTask;

	if (task == 0){
		return 0;
	}
	uint64_t value = task->timeLeft;
	while (task != 0){
		if ((value > task->timeLeft) && (task->timeLeft != 0)){
			value = task->timeLeft;
		}
		task = task->nextTask;
	}
	return value;
}

void MTR_Planner_reset(Task_t const * task){
	((Task_t*) (task))->timeLeft = task->timeout;
}

void MTR_Planner_pause(Task_t const * task){
	((Task_t*) (task))->enabled = false;
}

void MTR_Planner_resume(Task_t const * task){
	((Task_t*) (task))->enabled = true;
}

void MTR_Planner_setTimeout(Task_t const* task, uint64_t timeout){
	((Task_t*) (task))->timeout = timeout;
}

void MTR_Planner_setTimeLeft(Task_t const* task, uint64_t timeLeft){
	((Task_t*) (task))->timeLeft = timeLeft;
}

void MTR_Planner_setPlannerTypeState(TypeTask_t newTypeTask, bool_t state) {
	Planner.runTypeTask = (uint8_t) (Planner.runTypeTask & ~ (uint8_t) (1 << newTypeTask)) | (uint8_t) (state << newTypeTask);
}

void MTR_Planner_setTaskTypeState(Task_t* task, TypeTask_t newtypeState) {
	task->typeTask = newtypeState;
}

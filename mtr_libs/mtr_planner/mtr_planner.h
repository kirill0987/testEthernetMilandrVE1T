#ifndef MTR_PLANNER_H_
#define MTR_PLANNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mtr_service/mtr_service.h"

#ifndef CONFIG_PLANNER_TIMER
#define CONFIG_PLANNER_TIMER TIMER2
#endif

#ifndef CONFIG_PLANNER_TIMER_IRQ
#define CONFIG_PLANNER_TIMER_IRQ Timer2_IRQHandler
#endif

#ifndef CONFIG_PLANNER_FORCE_RUN_TASK_COUNTER
#define CONFIG_PLANNER_FORCE_RUN_TASK_COUNTER 100
#endif

typedef enum
{
	ControlTask,
	UpdateTask,
	NetworkTask,
	UsualTask

}TypeTask_t;

typedef struct{
	//its chain func with task
	void (*callback) () ;
	uint64_t timeout;
	uint64_t timeLeft;
	//void * = Task_t *
	void* nextTask;
	bool_t enabled;
	TypeTask_t typeTask;
}Task_t;


typedef struct{
	Task_t* firstTask;
	Task_t* lastTask;
	int32_t forceRunTaskCounter;
	uint8_t runTypeTask;
	uint32_t timeAlreadyCounted;
}Planner_t;


void MTR_Planner_runTasks();
void MTR_Planner_init();

Task_t const * MTR_Planner_addTask(void (*function)(), uint64_t msTimeout, bool_t enabled, TypeTask_t taskType);
Task_t const * MTR_Planner_addMicroTask(void (*function)(), uint64_t microsTimeout, bool_t enabled, TypeTask_t taskType);

Task_t const * MTR_Planner_getTask(void (*function)());

void MTR_Planner_reset(Task_t const* task);
void MTR_Planner_pause(Task_t const* task);
void MTR_Planner_resume(Task_t const* task);
void MTR_Planner_setTimeout(Task_t const* task, uint64_t timeout);
void MTR_Planner_setTimeLeft(Task_t const* task, uint64_t timeLeft);

void MTR_Planner_setPlannerTypeState(TypeTask_t newTypeTask_t, bool_t state);
void MTR_Planner_setTaskTypeState(Task_t* task, TypeTask_t newtypeState);

#ifdef __cplusplus
}
#endif

#endif /*MTR_PLANNER_H_ */

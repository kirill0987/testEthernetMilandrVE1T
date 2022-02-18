#include "DataManager.h"
#include "tasks.h"

#include "mtr_service/mtr_service.h"
#include "mtr_planner/mtr_planner.h"

float getFloatValue(uint16_t dataHi, uint16_t dataLo){

	uint16_t floatBuffer[2];
	floatBuffer[0] = dataHi;
	floatBuffer[1] = dataLo;
	return (*((float*) floatBuffer));
}

DataManager::DataManager(): BaseDataManager(){}

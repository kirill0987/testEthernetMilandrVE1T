#include "BaseDataManager.h"
#include "mtr_service/mtr_service.h"
#include "tasks.h"

static std::vector<BaseSomeValue*> inputDatas;
static std::vector<BaseSomeValue*> outputDatas;

std::vector<BaseSomeValue*>& BaseDataManager::inputData(){
	return inputDatas;
}

std::vector<BaseSomeValue*>& BaseDataManager::outputData(){
	return outputDatas;
}

BaseDataManager::BaseDataManager():
	_started {true}
{
	outputsBools.reset();
	outputsBoolsResetStatuses.reset();
	inputBools.reset();
}


template <typename T>
static BaseSomeValue* registerInputValue(T value){
	static_assert(std::is_arithmetic<T>::value ||
			std::is_pointer<T>::value
			);

	if constexpr(std::is_pointer<T>::value) {
		static_assert( std::is_arithmetic<std::remove_pointer<T>>::value);
	}


	auto someValue = new DValue<T>(value);
	inputDatas.push_back(someValue);
	return someValue;
}


template <typename T>
static BaseSomeValue* registerOutputValue(T* value){

	static_assert(std::is_arithmetic<T>::value ||
			std::is_pointer<T>::value
			);

	if constexpr(std::is_pointer<T>::value) {
		static_assert( std::is_arithmetic<std::remove_pointer<T>>::value);
	}

	auto someValue = new DValue<T>(value);
	outputDatas.push_back(someValue);
	return someValue;
}

void BaseDataManager::setOutputBoolResetValue(uint32_t index, bool value) {
	outputsBoolsResetStatuses[index] = value;
}


void BaseDataManager::resetOutputs(uint32_t index)
{
	if (index == 0xFFFFFFFF){
		outputsBools = outputsBoolsResetStatuses;
	}
	else {
		outputsBools[index] = outputsBoolsResetStatuses[index];
	}
}

bool BaseDataManager::started() const{
	return _started;
}

void BaseDataManager::set(AllBoolOutputs elem, bool state){
	outputsBools.set(static_cast<int>(elem), state);
}

void BaseDataManager::set(AllBoolInputs elem, bool state){
	inputBools.set(static_cast<int>(elem), state);
}

bool BaseDataManager::test(AllBoolOutputs elem){
	return outputsBools.test(static_cast<int>(elem));
}

bool BaseDataManager::test(AllBoolInputs elem){
	return inputBools.test(static_cast<int>(elem));
}


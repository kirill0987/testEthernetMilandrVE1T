#ifndef CLASSES_BASEDATAMANAGER_H_
#define CLASSES_BASEDATAMANAGER_H_

#include <bitset>
#include <vector>
#include <type_traits>

#include "dataenums.h"
#include <mtr_service/mtr_service.h>

struct BaseSomeValue{
	BaseSomeValue(uint32_t value, uint8_t size) :
		_value {value},
		_size {size}
	{}

protected:
	void setValue(uint32_t value) { _value = value;};
	//uint32_t value() const {return _value; };
	//size_t size() const {return _size; };

	uint32_t _value;
	uint8_t _size;
};

//T can be numeric or class pointer
template <typename T>
struct DValue : public BaseSomeValue{

	template<typename... Args, typename = std::enable_if<std::is_class<T>::value>>
	DValue(Args... args) : BaseSomeValue(new T(args...), sizeof(T*))
	{
	}

	template<typename = std::enable_if<(std::is_enum<T>{})>>
	DValue() : BaseSomeValue(0, sizeof(T))
	{
	}


	DValue() : BaseSomeValue(0, sizeof(T)){
		//static_assert(std::is_pointer<T>::value);
	}

//	auto value() const {
//		return _value;
//	}

	template<typename = std::enable_if<std::is_class<T>::value>>
	auto value() const {
		return *static_cast<T*>(_value);
	}

	auto const & value() const {
		return *((T*)(&_value));
	}

	auto & value() {
		return *((T*)(&_value));
	}


//		}else if (std::is_pointer<T>::value) {
//			static_assert(std::is_class<std::remove_pointer<T>::type>::value);
//			return static_cast<T>(_value);
//		}
//		else {
//			return static_cast<T>(_value);
//		}
//	}


	/*void setValue(T& newValue){
		///if constexpr(std::is_arithmetic<T>::value){
		_value = newValue;
			//return;
		//}
			else if constexpr(std::is_pointer<T>::value){
			static_assert(false, "Its pointer, setValue unaccessable");
			//static_assert(!std::is_class<std::remove_pointer<T>::value>::value);
		}
		else {
			static_assert(false, "Its unknown, setValue unaccessable");
		}
	}*/

	void setValue(T newValue){
		_value = newValue;
	}

};

class BaseCanNetwork;

class BaseDataManager {
public:
	BaseDataManager();
	~BaseDataManager(){};

	void checkIssues();
	bool started() const;
	void setOutputBoolResetValue(uint32_t index, bool value);
	void resetOutputs(uint32_t index = 0xFFFFFFFF);

	virtual void stopOperating(){};
	virtual void startOperating(){};

	template <typename T>
	static BaseSomeValue* registerInputValue(T value);

	template <typename T>
	static BaseSomeValue* registerOutputValue(T value);

	void set(AllBoolOutputs elem, bool state = true);
	void set(AllBoolInputs elem, bool state = true);

	bool test(AllBoolOutputs elem);
	bool test(AllBoolInputs elem);

private :

	friend BaseCanNetwork;
	static std::vector<BaseSomeValue*>& inputData();
	static std::vector<BaseSomeValue*>& outputData();

	std::bitset<AllBoolOutputs::DoEnd> outputsBools;
	std::bitset<AllBoolInputs::DiEnd> inputBools;

	std::bitset<AllBoolOutputs::DoEnd> outputsBoolsResetStatuses;

protected:
	bool _started;

public:
	typename decltype(outputsBools)::reference operator[](AllBoolOutputs index) {
		return outputsBools[index];
	}

	typename decltype(inputBools)::reference operator[](AllBoolInputs index) {
		return inputBools[index];
	}


};

#endif /* CLASSES_BASEDATAMANAGER_H_ */

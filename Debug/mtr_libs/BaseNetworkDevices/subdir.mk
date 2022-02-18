################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mtr_libs/BaseNetworkDevices/ModbusDevice.cpp \
../mtr_libs/BaseNetworkDevices/NetworkDevice.cpp 

OBJS += \
./mtr_libs/BaseNetworkDevices/ModbusDevice.o \
./mtr_libs/BaseNetworkDevices/NetworkDevice.o 

CPP_DEPS += \
./mtr_libs/BaseNetworkDevices/ModbusDevice.d \
./mtr_libs/BaseNetworkDevices/NetworkDevice.d 


# Each subdirectory must supply rules for building sources it contributes
mtr_libs/BaseNetworkDevices/%.o: ../mtr_libs/BaseNetworkDevices/%.cpp mtr_libs/BaseNetworkDevices/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m1 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"E:\GitKraken\testEthernetMilandrVE1T" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis_boot" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis_lib\inc" -I"E:\GitKraken\testEthernetMilandrVE1T\inc" -I"E:\GitKraken\testEthernetMilandrVE1T\mtr_libs" -std=gnu++17 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



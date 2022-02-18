################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../classes/NetworkDevicesRS/VideoModule.cpp 

OBJS += \
./classes/NetworkDevicesRS/VideoModule.o 

CPP_DEPS += \
./classes/NetworkDevicesRS/VideoModule.d 


# Each subdirectory must supply rules for building sources it contributes
classes/NetworkDevicesRS/%.o: ../classes/NetworkDevicesRS/%.cpp classes/NetworkDevicesRS/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m1 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"E:\GitKraken\testEthernetMilandrVE1T" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis_boot" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis_lib\inc" -I"E:\GitKraken\testEthernetMilandrVE1T\inc" -I"E:\GitKraken\testEthernetMilandrVE1T\mtr_libs" -std=gnu++17 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



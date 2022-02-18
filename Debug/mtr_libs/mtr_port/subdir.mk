################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mtr_libs/mtr_port/mtr_port.c 

OBJS += \
./mtr_libs/mtr_port/mtr_port.o 

C_DEPS += \
./mtr_libs/mtr_port/mtr_port.d 


# Each subdirectory must supply rules for building sources it contributes
mtr_libs/mtr_port/%.o: ../mtr_libs/mtr_port/%.c mtr_libs/mtr_port/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m1 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"E:\GitKraken\testEthernetMilandrVE1T" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis_boot" -I"E:\GitKraken\testEthernetMilandrVE1T\cmsis_lib\inc" -I"E:\GitKraken\testEthernetMilandrVE1T\inc" -I"E:\GitKraken\testEthernetMilandrVE1T\mtr_libs" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



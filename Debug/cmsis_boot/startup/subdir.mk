################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../cmsis_boot/startup/startup_MDR1986VE1T.S 

OBJS += \
./cmsis_boot/startup/startup_MDR1986VE1T.o 

S_UPPER_DEPS += \
./cmsis_boot/startup/startup_MDR1986VE1T.d 


# Each subdirectory must supply rules for building sources it contributes
cmsis_boot/startup/%.o: ../cmsis_boot/startup/%.S cmsis_boot/startup/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m1 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



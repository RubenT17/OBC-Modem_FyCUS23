################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/lsm6dso/lsm6dso.c \
../Core/lsm6dso/lsm6dso_reg.c 

OBJS += \
./Core/lsm6dso/lsm6dso.o \
./Core/lsm6dso/lsm6dso_reg.o 

C_DEPS += \
./Core/lsm6dso/lsm6dso.d \
./Core/lsm6dso/lsm6dso_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Core/lsm6dso/%.o Core/lsm6dso/%.su: ../Core/lsm6dso/%.c Core/lsm6dso/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L431xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-lsm6dso

clean-Core-2f-lsm6dso:
	-$(RM) ./Core/lsm6dso/lsm6dso.d ./Core/lsm6dso/lsm6dso.o ./Core/lsm6dso/lsm6dso.su ./Core/lsm6dso/lsm6dso_reg.d ./Core/lsm6dso/lsm6dso_reg.o ./Core/lsm6dso/lsm6dso_reg.su

.PHONY: clean-Core-2f-lsm6dso


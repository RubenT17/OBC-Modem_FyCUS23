################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/RT_ds18b20.c \
../Core/Src/aocs.c \
../Core/Src/bus_packet.c \
../Core/Src/gnss_nmea.c \
../Core/Src/intelli.c \
../Core/Src/lis2mdltr.c \
../Core/Src/lsm6dso.c \
../Core/Src/main.c \
../Core/Src/ms5607.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c \
../Core/Src/tf_packet.c 

OBJS += \
./Core/Src/RT_ds18b20.o \
./Core/Src/aocs.o \
./Core/Src/bus_packet.o \
./Core/Src/gnss_nmea.o \
./Core/Src/intelli.o \
./Core/Src/lis2mdltr.o \
./Core/Src/lsm6dso.o \
./Core/Src/main.o \
./Core/Src/ms5607.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o \
./Core/Src/tf_packet.o 

C_DEPS += \
./Core/Src/RT_ds18b20.d \
./Core/Src/aocs.d \
./Core/Src/bus_packet.d \
./Core/Src/gnss_nmea.d \
./Core/Src/intelli.d \
./Core/Src/lis2mdltr.d \
./Core/Src/lsm6dso.d \
./Core/Src/main.d \
./Core/Src/ms5607.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d \
./Core/Src/tf_packet.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L431xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/RT_ds18b20.cyclo ./Core/Src/RT_ds18b20.d ./Core/Src/RT_ds18b20.o ./Core/Src/RT_ds18b20.su ./Core/Src/aocs.cyclo ./Core/Src/aocs.d ./Core/Src/aocs.o ./Core/Src/aocs.su ./Core/Src/bus_packet.cyclo ./Core/Src/bus_packet.d ./Core/Src/bus_packet.o ./Core/Src/bus_packet.su ./Core/Src/gnss_nmea.cyclo ./Core/Src/gnss_nmea.d ./Core/Src/gnss_nmea.o ./Core/Src/gnss_nmea.su ./Core/Src/intelli.cyclo ./Core/Src/intelli.d ./Core/Src/intelli.o ./Core/Src/intelli.su ./Core/Src/lis2mdltr.cyclo ./Core/Src/lis2mdltr.d ./Core/Src/lis2mdltr.o ./Core/Src/lis2mdltr.su ./Core/Src/lsm6dso.cyclo ./Core/Src/lsm6dso.d ./Core/Src/lsm6dso.o ./Core/Src/lsm6dso.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/ms5607.cyclo ./Core/Src/ms5607.d ./Core/Src/ms5607.o ./Core/Src/ms5607.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su ./Core/Src/tf_packet.cyclo ./Core/Src/tf_packet.d ./Core/Src/tf_packet.o ./Core/Src/tf_packet.su

.PHONY: clean-Core-2f-Src


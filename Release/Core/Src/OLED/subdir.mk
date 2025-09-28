################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/OLED/add.c \
../Core/Src/OLED/delay.c \
../Core/Src/OLED/oled.c \
../Core/Src/OLED/oledStatus.c 

OBJS += \
./Core/Src/OLED/add.o \
./Core/Src/OLED/delay.o \
./Core/Src/OLED/oled.o \
./Core/Src/OLED/oledStatus.o 

C_DEPS += \
./Core/Src/OLED/add.d \
./Core/Src/OLED/delay.d \
./Core/Src/OLED/oled.d \
./Core/Src/OLED/oledStatus.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/OLED/%.o Core/Src/OLED/%.su Core/Src/OLED/%.cyclo: ../Core/Src/OLED/%.c Core/Src/OLED/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Include -I"D:/AD/hardware/power/F446/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/AD/hardware/power/F446/Core/Inc" -I"D:/AD/hardware/power/F446/Core/Inc/FlashDrv" -I"D:/AD/hardware/power/F446/Core/Inc/OLED" -I"D:/AD/hardware/power/F446/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/AD/hardware/power/F446/Drivers/CMSIS/Include" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-OLED

clean-Core-2f-Src-2f-OLED:
	-$(RM) ./Core/Src/OLED/add.cyclo ./Core/Src/OLED/add.d ./Core/Src/OLED/add.o ./Core/Src/OLED/add.su ./Core/Src/OLED/delay.cyclo ./Core/Src/OLED/delay.d ./Core/Src/OLED/delay.o ./Core/Src/OLED/delay.su ./Core/Src/OLED/oled.cyclo ./Core/Src/OLED/oled.d ./Core/Src/OLED/oled.o ./Core/Src/OLED/oled.su ./Core/Src/OLED/oledStatus.cyclo ./Core/Src/OLED/oledStatus.d ./Core/Src/OLED/oledStatus.o ./Core/Src/OLED/oledStatus.su

.PHONY: clean-Core-2f-Src-2f-OLED


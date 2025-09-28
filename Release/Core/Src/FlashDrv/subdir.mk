################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/FlashDrv/FlashDrv.c 

OBJS += \
./Core/Src/FlashDrv/FlashDrv.o 

C_DEPS += \
./Core/Src/FlashDrv/FlashDrv.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/FlashDrv/%.o Core/Src/FlashDrv/%.su Core/Src/FlashDrv/%.cyclo: ../Core/Src/FlashDrv/%.c Core/Src/FlashDrv/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Admin/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Include -I"D:/AD/hardware/power/F446/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/AD/hardware/power/F446/Core/Inc" -I"D:/AD/hardware/power/F446/Core/Inc/FlashDrv" -I"D:/AD/hardware/power/F446/Core/Inc/OLED" -I"D:/AD/hardware/power/F446/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/AD/hardware/power/F446/Drivers/CMSIS/Include" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-FlashDrv

clean-Core-2f-Src-2f-FlashDrv:
	-$(RM) ./Core/Src/FlashDrv/FlashDrv.cyclo ./Core/Src/FlashDrv/FlashDrv.d ./Core/Src/FlashDrv/FlashDrv.o ./Core/Src/FlashDrv/FlashDrv.su

.PHONY: clean-Core-2f-Src-2f-FlashDrv


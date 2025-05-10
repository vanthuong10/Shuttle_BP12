################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Communication/Ethernet/moongose/mongoose.c 

OBJS += \
./Communication/Ethernet/moongose/mongoose.o 

C_DEPS += \
./Communication/Ethernet/moongose/mongoose.d 


# Each subdirectory must supply rules for building sources it contributes
Communication/Ethernet/moongose/%.o Communication/Ethernet/moongose/%.su Communication/Ethernet/moongose/%.cyclo: ../Communication/Ethernet/moongose/%.c Communication/Ethernet/moongose/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I.././Communication/Ethernet/moongose -I.././Communication/Ethernet/userApplication -I.././Peripherals/adc -I.././Peripherals/Bms -I.././Peripherals/dac -I.././Peripherals/gpio -I.././Peripherals/Motor -I.././Peripherals/QR_Reader -I.././Communication/CAN -I../Application/Database -I.././Application/Signal -I.././External_Libs -I.././Peripherals/Hydraulic -I../Application/Main_Program/Manual -I.././Application/Main_Program/Auto -I.././Application/Update_Data -I.././Application/Main_Program/Safety -I../Peripherals/oled -I../External_Libs/Fonts -I../Peripherals/led -I../Application/Display -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Communication-2f-Ethernet-2f-moongose

clean-Communication-2f-Ethernet-2f-moongose:
	-$(RM) ./Communication/Ethernet/moongose/mongoose.cyclo ./Communication/Ethernet/moongose/mongoose.d ./Communication/Ethernet/moongose/mongoose.o ./Communication/Ethernet/moongose/mongoose.su

.PHONY: clean-Communication-2f-Ethernet-2f-moongose


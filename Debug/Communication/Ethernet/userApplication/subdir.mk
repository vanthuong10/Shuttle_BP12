################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Communication/Ethernet/userApplication/mongoose_fs.c \
../Communication/Ethernet/userApplication/mongoose_glue.c \
../Communication/Ethernet/userApplication/mongoose_impl.c 

OBJS += \
./Communication/Ethernet/userApplication/mongoose_fs.o \
./Communication/Ethernet/userApplication/mongoose_glue.o \
./Communication/Ethernet/userApplication/mongoose_impl.o 

C_DEPS += \
./Communication/Ethernet/userApplication/mongoose_fs.d \
./Communication/Ethernet/userApplication/mongoose_glue.d \
./Communication/Ethernet/userApplication/mongoose_impl.d 


# Each subdirectory must supply rules for building sources it contributes
Communication/Ethernet/userApplication/%.o Communication/Ethernet/userApplication/%.su Communication/Ethernet/userApplication/%.cyclo: ../Communication/Ethernet/userApplication/%.c Communication/Ethernet/userApplication/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I.././Communication/Ethernet/moongose -I.././Communication/Ethernet/userApplication -I.././Peripherals/adc -I.././Peripherals/Bms -I.././Peripherals/dac -I.././Peripherals/gpio -I.././Peripherals/Motor -I.././Peripherals/QR_Reader -I.././Communication/CAN -I../Application/Database -I.././Application/Signal -I.././External_Libs -I.././Peripherals/Hydraulic -I../Application/Main_Program/Manual -I.././Application/Main_Program/Auto -I.././Application/Update_Data -I.././Application/Main_Program/Safety -I../Peripherals/oled -I../External_Libs/Fonts -I../Peripherals/led -I../Application/Display -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Communication-2f-Ethernet-2f-userApplication

clean-Communication-2f-Ethernet-2f-userApplication:
	-$(RM) ./Communication/Ethernet/userApplication/mongoose_fs.cyclo ./Communication/Ethernet/userApplication/mongoose_fs.d ./Communication/Ethernet/userApplication/mongoose_fs.o ./Communication/Ethernet/userApplication/mongoose_fs.su ./Communication/Ethernet/userApplication/mongoose_glue.cyclo ./Communication/Ethernet/userApplication/mongoose_glue.d ./Communication/Ethernet/userApplication/mongoose_glue.o ./Communication/Ethernet/userApplication/mongoose_glue.su ./Communication/Ethernet/userApplication/mongoose_impl.cyclo ./Communication/Ethernet/userApplication/mongoose_impl.d ./Communication/Ethernet/userApplication/mongoose_impl.o ./Communication/Ethernet/userApplication/mongoose_impl.su

.PHONY: clean-Communication-2f-Ethernet-2f-userApplication


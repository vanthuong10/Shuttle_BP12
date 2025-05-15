################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Peripherals/oled/GUI_Paint.c \
../Peripherals/oled/ImageData.c \
../Peripherals/oled/Oled_Config.c \
../Peripherals/oled/Oled_PalletShuttle.c 

OBJS += \
./Peripherals/oled/GUI_Paint.o \
./Peripherals/oled/ImageData.o \
./Peripherals/oled/Oled_Config.o \
./Peripherals/oled/Oled_PalletShuttle.o 

C_DEPS += \
./Peripherals/oled/GUI_Paint.d \
./Peripherals/oled/ImageData.d \
./Peripherals/oled/Oled_Config.d \
./Peripherals/oled/Oled_PalletShuttle.d 


# Each subdirectory must supply rules for building sources it contributes
Peripherals/oled/%.o Peripherals/oled/%.su Peripherals/oled/%.cyclo: ../Peripherals/oled/%.c Peripherals/oled/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I.././Communication/Ethernet/moongose -I.././Communication/Ethernet/userApplication -I.././Peripherals/adc -I.././Peripherals/Bms -I.././Peripherals/dac -I.././Peripherals/gpio -I.././Peripherals/Motor -I.././Peripherals/QR_Reader -I.././Communication/CAN -I../Application/Database -I.././Application/Signal -I.././External_Libs -I.././Peripherals/Hydraulic -I../Application/Main_Program/Manual -I.././Application/Main_Program/Auto -I.././Application/Update_Data -I.././Application/Main_Program/Safety -I../Peripherals/oled -I../Peripherals/led -I../Application/Display -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Peripherals-2f-oled

clean-Peripherals-2f-oled:
	-$(RM) ./Peripherals/oled/GUI_Paint.cyclo ./Peripherals/oled/GUI_Paint.d ./Peripherals/oled/GUI_Paint.o ./Peripherals/oled/GUI_Paint.su ./Peripherals/oled/ImageData.cyclo ./Peripherals/oled/ImageData.d ./Peripherals/oled/ImageData.o ./Peripherals/oled/ImageData.su ./Peripherals/oled/Oled_Config.cyclo ./Peripherals/oled/Oled_Config.d ./Peripherals/oled/Oled_Config.o ./Peripherals/oled/Oled_Config.su ./Peripherals/oled/Oled_PalletShuttle.cyclo ./Peripherals/oled/Oled_PalletShuttle.d ./Peripherals/oled/Oled_PalletShuttle.o ./Peripherals/oled/Oled_PalletShuttle.su

.PHONY: clean-Peripherals-2f-oled


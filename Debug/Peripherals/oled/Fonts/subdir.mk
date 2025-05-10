################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Peripherals/oled/Fonts/font11.c \
../Peripherals/oled/Fonts/font12.c \
../Peripherals/oled/Fonts/font13.c \
../Peripherals/oled/Fonts/font15.c \
../Peripherals/oled/Fonts/font16.c \
../Peripherals/oled/Fonts/font20.c \
../Peripherals/oled/Fonts/font24.c \
../Peripherals/oled/Fonts/font27.c 

OBJS += \
./Peripherals/oled/Fonts/font11.o \
./Peripherals/oled/Fonts/font12.o \
./Peripherals/oled/Fonts/font13.o \
./Peripherals/oled/Fonts/font15.o \
./Peripherals/oled/Fonts/font16.o \
./Peripherals/oled/Fonts/font20.o \
./Peripherals/oled/Fonts/font24.o \
./Peripherals/oled/Fonts/font27.o 

C_DEPS += \
./Peripherals/oled/Fonts/font11.d \
./Peripherals/oled/Fonts/font12.d \
./Peripherals/oled/Fonts/font13.d \
./Peripherals/oled/Fonts/font15.d \
./Peripherals/oled/Fonts/font16.d \
./Peripherals/oled/Fonts/font20.d \
./Peripherals/oled/Fonts/font24.d \
./Peripherals/oled/Fonts/font27.d 


# Each subdirectory must supply rules for building sources it contributes
Peripherals/oled/Fonts/%.o Peripherals/oled/Fonts/%.su Peripherals/oled/Fonts/%.cyclo: ../Peripherals/oled/Fonts/%.c Peripherals/oled/Fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I.././Communication/Ethernet/moongose -I.././Communication/Ethernet/userApplication -I.././Peripherals/adc -I.././Peripherals/Bms -I.././Peripherals/dac -I.././Peripherals/gpio -I.././Peripherals/Motor -I.././Peripherals/QR_Reader -I.././Communication/CAN -I../Application/Database -I.././Application/Signal -I.././External_Libs -I.././Peripherals/Hydraulic -I../Application/Main_Program/Manual -I.././Application/Main_Program/Auto -I.././Application/Update_Data -I.././Application/Main_Program/Safety -I../Peripherals/oled -I../Peripherals/oled/Fonts -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Peripherals-2f-oled-2f-Fonts

clean-Peripherals-2f-oled-2f-Fonts:
	-$(RM) ./Peripherals/oled/Fonts/font11.cyclo ./Peripherals/oled/Fonts/font11.d ./Peripherals/oled/Fonts/font11.o ./Peripherals/oled/Fonts/font11.su ./Peripherals/oled/Fonts/font12.cyclo ./Peripherals/oled/Fonts/font12.d ./Peripherals/oled/Fonts/font12.o ./Peripherals/oled/Fonts/font12.su ./Peripherals/oled/Fonts/font13.cyclo ./Peripherals/oled/Fonts/font13.d ./Peripherals/oled/Fonts/font13.o ./Peripherals/oled/Fonts/font13.su ./Peripherals/oled/Fonts/font15.cyclo ./Peripherals/oled/Fonts/font15.d ./Peripherals/oled/Fonts/font15.o ./Peripherals/oled/Fonts/font15.su ./Peripherals/oled/Fonts/font16.cyclo ./Peripherals/oled/Fonts/font16.d ./Peripherals/oled/Fonts/font16.o ./Peripherals/oled/Fonts/font16.su ./Peripherals/oled/Fonts/font20.cyclo ./Peripherals/oled/Fonts/font20.d ./Peripherals/oled/Fonts/font20.o ./Peripherals/oled/Fonts/font20.su ./Peripherals/oled/Fonts/font24.cyclo ./Peripherals/oled/Fonts/font24.d ./Peripherals/oled/Fonts/font24.o ./Peripherals/oled/Fonts/font24.su ./Peripherals/oled/Fonts/font27.cyclo ./Peripherals/oled/Fonts/font27.d ./Peripherals/oled/Fonts/font27.o ./Peripherals/oled/Fonts/font27.su

.PHONY: clean-Peripherals-2f-oled-2f-Fonts


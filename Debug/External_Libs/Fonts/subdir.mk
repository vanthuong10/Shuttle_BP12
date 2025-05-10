################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../External_Libs/Fonts/font11.c \
../External_Libs/Fonts/font12.c \
../External_Libs/Fonts/font13.c \
../External_Libs/Fonts/font15.c \
../External_Libs/Fonts/font16.c \
../External_Libs/Fonts/font20.c \
../External_Libs/Fonts/font24.c \
../External_Libs/Fonts/font27.c 

OBJS += \
./External_Libs/Fonts/font11.o \
./External_Libs/Fonts/font12.o \
./External_Libs/Fonts/font13.o \
./External_Libs/Fonts/font15.o \
./External_Libs/Fonts/font16.o \
./External_Libs/Fonts/font20.o \
./External_Libs/Fonts/font24.o \
./External_Libs/Fonts/font27.o 

C_DEPS += \
./External_Libs/Fonts/font11.d \
./External_Libs/Fonts/font12.d \
./External_Libs/Fonts/font13.d \
./External_Libs/Fonts/font15.d \
./External_Libs/Fonts/font16.d \
./External_Libs/Fonts/font20.d \
./External_Libs/Fonts/font24.d \
./External_Libs/Fonts/font27.d 


# Each subdirectory must supply rules for building sources it contributes
External_Libs/Fonts/%.o External_Libs/Fonts/%.su External_Libs/Fonts/%.cyclo: ../External_Libs/Fonts/%.c External_Libs/Fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I.././Communication/Ethernet/moongose -I.././Communication/Ethernet/userApplication -I.././Peripherals/adc -I.././Peripherals/Bms -I.././Peripherals/dac -I.././Peripherals/gpio -I.././Peripherals/Motor -I.././Peripherals/QR_Reader -I.././Communication/CAN -I../Application/Database -I.././Application/Signal -I.././External_Libs -I.././Peripherals/Hydraulic -I../Application/Main_Program/Manual -I.././Application/Main_Program/Auto -I.././Application/Update_Data -I.././Application/Main_Program/Safety -I../Peripherals/oled -I../External_Libs/Fonts -I../Peripherals/led -I../Application/Display -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-External_Libs-2f-Fonts

clean-External_Libs-2f-Fonts:
	-$(RM) ./External_Libs/Fonts/font11.cyclo ./External_Libs/Fonts/font11.d ./External_Libs/Fonts/font11.o ./External_Libs/Fonts/font11.su ./External_Libs/Fonts/font12.cyclo ./External_Libs/Fonts/font12.d ./External_Libs/Fonts/font12.o ./External_Libs/Fonts/font12.su ./External_Libs/Fonts/font13.cyclo ./External_Libs/Fonts/font13.d ./External_Libs/Fonts/font13.o ./External_Libs/Fonts/font13.su ./External_Libs/Fonts/font15.cyclo ./External_Libs/Fonts/font15.d ./External_Libs/Fonts/font15.o ./External_Libs/Fonts/font15.su ./External_Libs/Fonts/font16.cyclo ./External_Libs/Fonts/font16.d ./External_Libs/Fonts/font16.o ./External_Libs/Fonts/font16.su ./External_Libs/Fonts/font20.cyclo ./External_Libs/Fonts/font20.d ./External_Libs/Fonts/font20.o ./External_Libs/Fonts/font20.su ./External_Libs/Fonts/font24.cyclo ./External_Libs/Fonts/font24.d ./External_Libs/Fonts/font24.o ./External_Libs/Fonts/font24.su ./External_Libs/Fonts/font27.cyclo ./External_Libs/Fonts/font27.d ./External_Libs/Fonts/font27.o ./External_Libs/Fonts/font27.su

.PHONY: clean-External_Libs-2f-Fonts


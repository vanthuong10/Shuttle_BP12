################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Peripherals/led/subdir.mk
-include Peripherals/gpio/subdir.mk
-include Peripherals/adc/subdir.mk
-include Peripherals/QR_Reader/subdir.mk
-include Peripherals/Motor/subdir.mk
-include Peripherals/Hydraulic/subdir.mk
-include Peripherals/Bms/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include External_Libs/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Communication/Ethernet/userApplication/subdir.mk
-include Communication/Ethernet/moongose/subdir.mk
-include Communication/CAN/subdir.mk
-include Application/Update_Data/subdir.mk
-include Application/Signal/subdir.mk
-include Application/Main_Program/Safety/subdir.mk
-include Application/Main_Program/Manual/subdir.mk
-include Application/Main_Program/Auto/subdir.mk
-include Application/Display/subdir.mk
-include Application/Database/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ShuttleProgram_0.1.1
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
ShuttleProgram_0.1.1.elf \

MAP_FILES += \
ShuttleProgram_0.1.1.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
ShuttleProgram_0.1.1.list \

OBJCOPY_BIN += \
ShuttleProgram_0.1.1.bin \


# All Target
all: main-build

# Main-build Target
main-build: ShuttleProgram_0.1.1.elf secondary-outputs

# Tool invocations
ShuttleProgram_0.1.1.elf ShuttleProgram_0.1.1.map: $(OBJS) $(USER_OBJS) E:\Project\Pallet\ Shuttle\4.Program\Done\ShuttleV1_1\STM32H743XIHX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "ShuttleProgram_0.1.1.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m7 -T"E:\Project\Pallet Shuttle\4.Program\Done\ShuttleV1_1\STM32H743XIHX_FLASH.ld" --specs=nosys.specs -Wl,-Map="ShuttleProgram_0.1.1.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -u _scanf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ShuttleProgram_0.1.1.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ShuttleProgram_0.1.1.list"
	@echo 'Finished building: $@'
	@echo ' '

ShuttleProgram_0.1.1.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "ShuttleProgram_0.1.1.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ShuttleProgram_0.1.1.bin ShuttleProgram_0.1.1.elf ShuttleProgram_0.1.1.list ShuttleProgram_0.1.1.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets

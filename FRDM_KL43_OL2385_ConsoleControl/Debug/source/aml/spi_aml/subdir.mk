################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/aml/spi_aml/spi_aml.c 

OBJS += \
./source/aml/spi_aml/spi_aml.o 

C_DEPS += \
./source/aml/spi_aml/spi_aml.d 


# Each subdirectory must supply rules for building sources it contributes
source/aml/spi_aml/%.o: ../source/aml/spi_aml/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MKL43Z256VMP4" -I../startup -I../source -I../board -I../utilities -I../CMSIS -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/sf/sf.c \
../source/sf/sf_setup.c 

OBJS += \
./source/sf/sf.o \
./source/sf/sf_setup.o 

C_DEPS += \
./source/sf/sf.d \
./source/sf/sf_setup.d 


# Each subdirectory must supply rules for building sources it contributes
source/sf/%.o: ../source/sf/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MKL43Z256VMP4" -I../startup -I../source -I../board -I../utilities -I../CMSIS -I../drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



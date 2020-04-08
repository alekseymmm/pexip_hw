################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../geometry.c \
../img_operations.c \
../jpeg_utils.c \
../main.c 

OBJS += \
./geometry.o \
./img_operations.o \
./jpeg_utils.o \
./main.o 

C_DEPS += \
./geometry.d \
./img_operations.d \
./jpeg_utils.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



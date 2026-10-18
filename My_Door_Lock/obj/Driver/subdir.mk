################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Driver/as608.c \
../Driver/esp8266.c \
../Driver/key.c \
../Driver/lcd.c \
../Driver/scheduler.c \
../Driver/timer.c \
../Driver/uart.c \
../Driver/voice.c 

OBJS += \
./Driver/as608.o \
./Driver/esp8266.o \
./Driver/key.o \
./Driver/lcd.o \
./Driver/scheduler.o \
./Driver/timer.o \
./Driver/uart.o \
./Driver/voice.o 

C_DEPS += \
./Driver/as608.d \
./Driver/esp8266.d \
./Driver/key.d \
./Driver/lcd.d \
./Driver/scheduler.d \
./Driver/timer.d \
./Driver/uart.d \
./Driver/voice.d 


# Each subdirectory must supply rules for building sources it contributes
Driver/%.o: ../Driver/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"E:\2_Code\CH32\My_Door_Lock\Debug" -I"E:\2_Code\CH32\My_Door_Lock\Core" -I"E:\2_Code\CH32\My_Door_Lock\User" -I"E:\2_Code\CH32\My_Door_Lock\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@


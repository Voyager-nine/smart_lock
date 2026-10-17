################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Debug/debug.c 

OBJS += \
./Debug/debug.o 

C_DEPS += \
./Debug/debug.d 


# Each subdirectory must supply rules for building sources it contributes
Debug/%.o: ../Debug/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"E:\2_Code\CH32\智能门锁\V6.4\Debug" -I"E:\2_Code\CH32\智能门锁\V6.4\Core" -I"E:\2_Code\CH32\智能门锁\V6.4\User" -I"E:\2_Code\CH32\智能门锁\V6.4\Peripheral\inc" -I"D:\file\03嵌入式\代码\智能门锁部分\V4.20\Driver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@


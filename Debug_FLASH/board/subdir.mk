################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/adc_driver.c \
../board/clock_config.c \
../board/edma_driver.c \
../board/edma_hw_access.c \
../board/edma_irq.c \
../board/list.c \
../board/lpi2c_driver.c \
../board/lpi2c_hw_access.c \
../board/lpi2c_irq.c \
../board/osif_baremetal.c \
../board/peripherals_edma_config_1.c \
../board/peripherals_lpi2c_config_1.c \
../board/peripherals_osif_1.c \
../board/pin_mux.c 

OBJS += \
./board/adc_driver.o \
./board/clock_config.o \
./board/edma_driver.o \
./board/edma_hw_access.o \
./board/edma_irq.o \
./board/list.o \
./board/lpi2c_driver.o \
./board/lpi2c_hw_access.o \
./board/lpi2c_irq.o \
./board/osif_baremetal.o \
./board/peripherals_edma_config_1.o \
./board/peripherals_lpi2c_config_1.o \
./board/peripherals_osif_1.o \
./board/pin_mux.o 

C_DEPS += \
./board/adc_driver.d \
./board/clock_config.d \
./board/edma_driver.d \
./board/edma_hw_access.d \
./board/edma_irq.d \
./board/list.d \
./board/lpi2c_driver.d \
./board/lpi2c_hw_access.d \
./board/lpi2c_irq.d \
./board/osif_baremetal.d \
./board/peripherals_edma_config_1.d \
./board/peripherals_lpi2c_config_1.d \
./board/peripherals_osif_1.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@board/adc_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



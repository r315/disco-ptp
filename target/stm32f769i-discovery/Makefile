######################################
# target
######################################
TARGET =ptp-server

RELEASE =no
#######################################
# Features
#######################################
FEATURES += ENABLE_UART
FEATURES += ENABLE_UART_CMSIS_OS
FEATURES += ENABLE_CLI
FEATURES += ENABLE_CLI_COLOR
FEATURES += ENABLE_DHCP
#FEATURES += ENABLE_DHCP_SERVER
#FEATURES += ENABLE_LOG_TO_DISPLAY


#######################################
# paths
#######################################

BUILD_DIR 		:=build
TARGET_PATH 	=$(CURDIR)

APP_PATH        =$(TARGET_PATH)/../../app
MIDDLEWARE_PATH =$(TARGET_PATH)/../../Middlewares
DRIVERS_PATH 	=$(TARGET_PATH)/Drivers
CMSIS_PATH 		=$(DRIVERS_PATH)/CMSIS
DRIVERS_SOC     =$(DRIVERS_PATH)/STM32F7xx_HAL_Driver
DRIVERS_BSP     =$(DRIVERS_PATH)/BSP/STM32F769I_Discovery
UTILITIES_PATH  =$(TARGET_PATH)/../Utilities

#######################################
# Includes
#######################################
C_INCLUDES = \
$(CMSIS_PATH) \
$(DRIVERS_SOC)/Inc \
$(TARGET_PATH)/inc \
$(DRIVERS_BSP) \
$(APP_PATH)/inc \
$(APP_PATH)/src \
$(MIDDLEWARE_PATH)/LwIP/src/include \
$(MIDDLEWARE_PATH)/LwIP/system \
$(MIDDLEWARE_PATH)/LwIP/system/arch \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/include \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 \
$(MIDDLEWARE_PATH)/cli-simple/ \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src \
$(UTILITIES_PATH)/Log \

AS_INCLUDES =\

######################################
# Sources
######################################
DRIVER_SOURCES = \
$(DRIVERS_BSP)/stm32f769i_discovery.c \
$(DRIVERS_BSP)/stm32f769i_discovery_lcd.c \
$(DRIVERS_BSP)/stm32f769i_discovery_sdram.c \
$(DRIVERS_BSP)/stm32f769i_discovery_ts.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_cortex.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_dma.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_dma2d.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_dsi.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_eth.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_flash.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_gpio.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_i2c.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_ltdc.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_ltdc_ex.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_pwr.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_pwr_ex.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_sdram.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_rcc.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_rcc_ex.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_tim.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_tim_ex.c \
$(DRIVERS_SOC)/Src/stm32f7xx_hal_uart.c \
$(DRIVERS_SOC)/Src/stm32f7xx_ll_fmc.c \

COMPONENT_SOURCES = \
$(DRIVERS_BSP)/../Components/otm8009a/otm8009a.c \
$(UTILITIES_PATH)/Log/lcd_log.c \

FREERTOS_SOURCES = \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/portable/MemMang/heap_4.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/croutine.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/list.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/queue.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/tasks.c \
$(MIDDLEWARE_PATH)/FreeRTOS/Source/timers.c \

LWIP_SOURCES = \
$(wildcard $(MIDDLEWARE_PATH)/LwIP/src/core/*.c) \
$(wildcard $(MIDDLEWARE_PATH)/LwIP/src/core/ipv4/*.c) \
$(wildcard $(MIDDLEWARE_PATH)/LwIP/src/api/*.c) \
$(MIDDLEWARE_PATH)/LwIP/src/apps/http/fs.c \
$(MIDDLEWARE_PATH)/LwIP/src/apps/http/httpd.c \
$(MIDDLEWARE_PATH)/LwIP/src/apps/sntp/sntp.c \
$(MIDDLEWARE_PATH)/LwIP/src/netif/ethernet.c \
$(MIDDLEWARE_PATH)/LwIP/system/OS/sys_arch.c \

PTPD_SOURCES = \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/arith.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/bmc.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/ptpd.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/protocol.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/dep/sys_time.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/dep/msg.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/dep/net.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/dep/servo.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/dep/startup.c \
$(MIDDLEWARE_PATH)/ptpd-v2.0.0/src/dep/timer.c \

C_SOURCES = \
$(DRIVER_SOURCES) \
$(COMPONENT_SOURCES) \
$(FREERTOS_SOURCES) \
$(LWIP_SOURCES) \
$(PTPD_SOURCES) \
$(TARGET_PATH)/src/system_stm32f7xx.c \
$(TARGET_PATH)/src/stm32f7xx_uart.c \
$(TARGET_PATH)/src/stm32f7xx_hal_timebase_tim.c \
$(TARGET_PATH)/src/stm32f7xx_it.c \
$(TARGET_PATH)/src/syscalls.c \
$(TARGET_PATH)/src/main.c \
$(MIDDLEWARE_PATH)/cli-simple/cli_simple.c \
$(APP_PATH)/src/ethernetif.c \
$(APP_PATH)/src/httpserver-socket.c \
$(APP_PATH)/src/dhcp_server.c \
$(APP_PATH)/src/app_ethernet.c \
$(APP_PATH)/src/app.c \
$(APP_PATH)/src/ping.c \

CPP_SOURCES = \

ASM_SOURCES = \
$(TARGET_PATH)/startup_stm32f769xx.s

LDSCRIPT :=$(TARGET_PATH)/STM32F769NIHx_FLASH.ld

#######################################
# Misc
#######################################

ifeq ($(RELEASE),yes)
OPT   =-O2
DEBUG =DEBUG=0
GIT_TAG :=$(shell git describe --abbrev=0 --tags 2>/dev/null || true)
VERSION :=VERSION=\"$(GIT_TAG)\"
else
OPT   =-O0 -g -gdwarf-2 -Wall
DEBUG =DEBUG=1
endif

# C defines
C_DEFS +=\
$(DEVICE) \
$(FEATURES) \
USE_HAL_DRIVER \
USE_STM32F769I_DISCO \
$(DEBUG) \
$(VERSION) \


#######################################
# CFLAGS
#######################################

DEVICE =STM32F769xx

# cpu
CPU =-mcpu=cortex-m7 -mthumb

# fpu
FPU =-mfloat-abi=hard -mfpu=fpv5-d16

# float-abi
FLOAT_ABI =#-u_printf_float

# mcu
MCU = $(CPU) $(FPU) $(FLOAT_ABI)

ifndef V
VERBOSE =@
else
VERBOSE =
endif

# Generate dependency information
#CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))
#
ASFLAGS  =$(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -fdata-sections -ffunction-sections

CFLAGS   =$(MCU) $(OPT) $(addprefix -D, $(C_DEFS)) $(addprefix -I, $(C_INCLUDES)) -std=gnu11 -fdata-sections -ffunction-sections
#-fstack-usage
CPPFLAGS =$(CPU) $(OPT) $(addprefix -D, $(C_DEFS)) $(addprefix -I, $(C_INCLUDES)) -std=c++11 -fdata-sections -ffunction-sections -fno-unwind-tables -fno-exceptions -fno-rtti

LDFLAGS  =$(MCU) $(SPECS) -Wl,--gc-sections,-Map=$(BUILD_DIR)/$(TARGET).map,--cref

#######################################
# Libraries
#######################################

LIBDIR =
ifdef SEMIHOSTING
SPECS =--specs=rdimon.specs
LDLIBS =-nostartfiles -lc -lrdimon
else
SPECS=--specs=nosys.specs --specs=nano.specs
LIBS =-lstdc++
endif

#######################################
# Objects
#######################################

OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CPP_SOURCES:.cpp=.obj)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

#######################################
# Tool binaries
#######################################
PREFIX = arm-none-eabi-
CC  =$(PREFIX)gcc
CPP =$(PREFIX)g++
AS  =$(PREFIX)gcc -x assembler-with-cpp
LD  =$(PREFIX)gcc
AR  =$(PREFIX)ar
SZ  =$(PREFIX)size
GDB =$(PREFIX)gdb
OBJCOPY =$(PREFIX)objcopy
OBJDUMP =$(PREFIX)objdump
HEX = $(OBJCOPY) -O ihex
BIN = $(OBJCOPY) -O binary -S

#######################################
# Rules
#######################################
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).elf size
#@echo $(CMSIS_PATH)

bin: $(BUILD_DIR)/$(TARGET).bin

program: $(BUILD_DIR)/$(TARGET).bin
	openocd -f discovery.cfg -c "program $(BUILD_DIR)/$(TARGET).bin 0x08000000 verify reset exit"

#######################################
# build rules
#######################################
$(BUILD_DIR)/%.d: %.c
	$(VERBOSE)$(CC) $(CFLAGS) -MF"$@" -MG -MM -MP -MT"$@" -MT"$(<:.c=.o)" "$<"

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]  $<"
	$(VERBOSE)$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.obj: %.cpp Makefile | $(BUILD_DIR)
	@echo "[CP]  $<"
	$(VERBOSE)$(CPP) -c $(CPPFLAGS)  $< -o $@

$(BUILD_DIR)/%.o: %.s
	@echo "[AS]  $<"
	$(VERBOSE)$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "[LD]  $@"
	$(VERBOSE)$(LD) $(LIBDIR) -T$(LDSCRIPT) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(VERBOSE)$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(VERBOSE)$(OBJCOPY) -O binary -R .RxDescripSection -R .TxDescripSection -R .RxBUF -R .TxBUF $< $@

$(BUILD_DIR):
	mkdir -p $@

size: $(BUILD_DIR)/$(TARGET).elf
	@echo "--- Size ---"
	$(VERBOSE)$(SZ) -A -x $<
	$(VERBOSE)$(SZ) -B $<

#######################################
# clean up
#######################################
clean:
	$(VERBOSE)-rm -fR $(BUILD_DIR)

# *** EOF ***

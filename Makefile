########################################################################
# Start of user section
#

# Default var
SPECIFIC_REV   = 

ROOT_DIR = .
#d:/wyres/code/BLE_V2
# Name of output file
OUTPUT_DIR     = ./outputs
OUTPUT_FILE    = ble_modem
OUTPUT_DIRFILE = $(OUTPUT_DIR)/$(OUTPUT_FILE)

# Directories where find config files for tools
CONFIGDIR = tools
OPENOCD = "c:/soft/openocd-0.10.0"
OPENOCD_SCRIPT = "$(OPENOCD)/share/openocd/scripts"
MKDIR = "mkdir.exe"

# GNU parameters - if gcc is on your path then leave GNU_INSTALL_ROOT empty
#C:\Program Files (x86)\GNU Tools ARM Embedded\8 2019-q3-update
#GNU_INSTALL_ROOT := $(ROOT_DIR)/GNU_Tools_Arm_Embedded/7_2018-q2-update/bin/
GNU_INSTALL_ROOT := 
GNU_PREFIX := arm-none-eabi


# Toolchain
#TARGET  = arm-none-eabi-
#CC      = $(TARGET)gcc
#OBJCOPY = $(TARGET)objcopy
#AS      = $(TARGET)as
#SIZE    = $(TARGET)size
#OBJDUMP = $(TARGET)objdump
#GDB 	= $(TARGET)gdb

# Toolchain commands
CC      := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-gcc"
CXX     := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-c++"
AS      := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-as"
AR      := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-ar" -r
LD      := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-ld"
NM      := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-nm"
OBJDUMP := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-objdump"
OBJCOPY := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-objcopy"
SIZE    := "$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-size"
$(if $(shell $(CC) --version),,$(info Cannot find: $(CC).) \
  $(info Please set values in: "$(abspath $(TOOLCHAIN_CONFIG_FILE))") \
  $(info according to the actual configuration of your system.) \
  $(error Cannot continue))


# The CPU used
MCU = cortex-m0
# Add -mthumb for use THUMB code
THUMB = -mthumb

#################################
# Start define flags
#

# User flags for ASM 
# UASFLAGS =

# Default flags for C 
UCFLAGS  += -Wall -std=c99
#UCFLAGS  += -fmessage-length=0 -fdata-sections -ffunction-sections
# Default flags for linkage. Note use of -nostdlib to avoid the standard GCC stdlibc (we are using baselibc)
ULDFLAGS = -Wl,-Map=$(OUTPUT_DIRFILE).map,-lm,-nostdlib
# ULDFLAGS = -lm -Wl,-Map=$(OUTPUT_DIRFILE).map,--gc-sections
#ULDFLAGS += -Wl,-Map=$(OUTPUT_DIRFILE).map,-static


# Release flags additional for ASM 
# RASFLAGS =
# Release flags additional for C 
RCFLAGS  = -Os -DRELEASE_BUILD
# RCFLAGS  += -fshort-enums
# RCFLAGS  += -fpack-struct >>> build failed
# RCFLAGS  = -O1 -g
# RCFLAGS  += -fno-threadsafe-statics
# RCFLAGS  += -ffunction-sections
# RCFLAGS  += -fdata-sections
# Release flags additional for linkage 
RLDFLAGS = 

# Debug flags additional for ASM 
DASFLAGS =
# Debug flags additional for C 
DCFLAGS  = -O0 -g3 -DDEBUG
# Debug flags additional for linkage 
DLDFLAGS = 

#
# End define flags
#################################

# Linker script file
LDSCRIPT = ./linker/nrf51_xxac.ld

# List ASM source files
ASRC = ./components/toolchain/gcc/gcc_startup_nrf51.s

# List C source files

C_SOURCE_PATH = ./src
CSRC += $(wildcard $(C_SOURCE_PATH)/*.c)
CSRC += $(wildcard ./baselibc/src/*.c)
CSRC += ./components/boards/boards.c
CSRC += ./components/toolchain/system_nrf51.c
CSRC += ./components/ble/common/ble_advdata.c
CSRC += ./components/ble/common/ble_conn_params.c
CSRC += ./components/ble/common/ble_srv_common.c
CSRC += ./components/ble/ble_db_discovery/ble_db_discovery.c
CSRC += ./components/ble/ble_advertising/ble_advertising.c
CSRC += ./components/ble/ble_services/ble_nus/ble_nus.c
CSRC += ./components/ble/ble_services/ble_nus_c/ble_nus_c.c
CSRC += ./components/drivers_nrf/clock/nrf_drv_clock.c
CSRC += ./components/drivers_nrf/common/nrf_drv_common.c
CSRC += ./components/drivers_nrf/gpiote/nrf_drv_gpiote.c
CSRC += ./components/drivers_nrf/uart/nrf_drv_uart.c
CSRC += ./components/libraries/button/app_button.c
CSRC += ./components/libraries/util/app_error.c
CSRC += ./components/libraries/util/app_error_weak.c
CSRC += ./components/libraries/fifo/app_fifo.c
CSRC += ./components/libraries/timer/app_timer.c
CSRC += ./components/libraries/uart/app_uart_fifo.c
CSRC += ./components/libraries/util/app_util_platform.c
CSRC += ./components/libraries/hardfault/hardfault_implementation.c
CSRC += ./components/libraries/util/nrf_assert.c
CSRC += ./components/libraries/uart/retarget.c
CSRC += ./components/libraries/util/sdk_errors.c
CSRC += ./components/libraries/fstorage/fstorage.c
CSRC += ./components/libraries/log/src/nrf_log_backend_serial.c
CSRC += ./components/libraries/log/src/nrf_log_frontend.c
# CSRC += ./external/segger_rtt/RTT_Syscalls_KEIL.c
# CSRC += ./external/segger_rtt/SEGGER_RTT.c
# CSRC += ./external/segger_rtt/SEGGER_RTT_printf.c
CSRC += ./components/softdevice/common/softdevice_handler/softdevice_handler.c
CSRC += ./components/libraries/bsp/bsp.c
CSRC += ./components/libraries/bsp/bsp_btn_ble.c

# List of directories to include
UINCDIR = ./includes
UINCDIR += ./baselibc/include
UINCDIR += ./baselibc/src/templates
UINCDIR += ./config
UINCDIR += ./GNU_Tools_Arm_Embedded/7_2018-q2-update/arm-none-eabi/include
UINCDIR += ./components
UINCDIR += ./components/ble/ble_advertising
UINCDIR += ./components/ble/ble_db_discovery
UINCDIR += ./components/ble/ble_dtm
UINCDIR += ./components/ble/ble_racp
UINCDIR += ./components/ble/ble_services/ble_ancs_c
UINCDIR += ./components/ble/ble_services/ble_ans_c
UINCDIR += ./components/ble/ble_services/ble_bas
UINCDIR += ./components/ble/ble_services/ble_bas_c
UINCDIR += ./components/ble/ble_services/ble_cscs
UINCDIR += ./components/ble/ble_services/ble_cts_c
UINCDIR += ./components/ble/ble_services/ble_dfu
UINCDIR += ./components/ble/ble_services/ble_dis
UINCDIR += ./components/ble/ble_services/ble_gls
UINCDIR += ./components/ble/ble_services/ble_hids
UINCDIR += ./components/ble/ble_services/ble_hrs
UINCDIR += ./components/ble/ble_services/ble_hrs_c
UINCDIR += ./components/ble/ble_services/ble_hts
UINCDIR += ./components/ble/ble_services/ble_ias
UINCDIR += ./components/ble/ble_services/ble_ias_c
UINCDIR += ./components/ble/ble_services/ble_lbs
UINCDIR += ./components/ble/ble_services/ble_lbs_c
UINCDIR += ./components/ble/ble_services/ble_lls
UINCDIR += ./components/ble/ble_services/ble_nus
UINCDIR += ./components/ble/ble_services/ble_nus_c
UINCDIR += ./components/ble/ble_services/ble_rscs
UINCDIR += ./components/ble/ble_services/ble_rscs_c
UINCDIR += ./components/ble/common
UINCDIR += ./components/ble/nrf_ble_qwr
UINCDIR += ./components/ble/peer_manager
UINCDIR += ./components/boards
UINCDIR += ./components/device
UINCDIR += ./components/drivers_nrf/adc
UINCDIR += ./components/drivers_nrf/clock
UINCDIR += ./components/drivers_nrf/common
UINCDIR += ./components/drivers_nrf/comp
UINCDIR += ./components/drivers_nrf/delay
UINCDIR += ./components/drivers_nrf/gpiote
UINCDIR += ./components/drivers_nrf/hal
UINCDIR += ./components/drivers_nrf/i2s
UINCDIR += ./components/drivers_nrf/lpcomp
UINCDIR += ./components/drivers_nrf/pdm
UINCDIR += ./components/drivers_nrf/powerppi
UINCDIR += ./components/drivers_nrf/pwm
UINCDIR += ./components/drivers_nrf/qdec
UINCDIR += ./components/drivers_nrf/rng
UINCDIR += ./components/drivers_nrf/rtc
UINCDIR += ./components/drivers_nrf/saadc
UINCDIR += ./components/drivers_nrf/spi_master
UINCDIR += ./components/drivers_nrf/spi_slave
UINCDIR += ./components/drivers_nrf/swi
UINCDIR += ./components/drivers_nrf/timer
UINCDIR += ./components/drivers_nrf/twi_master
UINCDIR += ./components/drivers_nrf/twis_slave
UINCDIR += ./components/drivers_nrf/uart
UINCDIR += ./components/drivers_nrf/usbd
UINCDIR += ./components/drivers_nrf/wdt
UINCDIR += ./components/libraries/bsp
UINCDIR += ./components/libraries/button
UINCDIR += ./components/libraries/crc16
UINCDIR += ./components/libraries/crc32
UINCDIR += ./components/libraries/csense
UINCDIR += ./components/libraries/csense_drv
UINCDIR += ./components/libraries/experimental_section_vars
UINCDIR += ./components/libraries/fds
UINCDIR += ./components/libraries/fifo
UINCDIR += ./components/libraries/fstorage
UINCDIR += ./components/libraries/gpiote
UINCDIR += ./components/libraries/hardfault
UINCDIR += ./components/libraries/hci
UINCDIR += ./components/libraries/led_softblink
UINCDIR += ./components/libraries/log
UINCDIR += ./components/libraries/log/src
UINCDIR += ./components/libraries/low_power_pwm
UINCDIR += ./components/libraries/mem_manager
UINCDIR += ./components/libraries/pwm
UINCDIR += ./components/libraries/queue
UINCDIR += ./components/libraries/scheduler
UINCDIR += ./components/libraries/slip
UINCDIR += ./components/libraries/timer
UINCDIR += ./components/libraries/twi
UINCDIR += ./components/libraries/uart
UINCDIR += ./components/libraries/usbd
UINCDIR += ./components/libraries/usbd/class/audio
UINCDIR += ./components/libraries/usbd/class/cdc
UINCDIR += ./components/libraries/usbd/class/cdc/acm
UINCDIR += ./components/libraries/usbd/class/hid
UINCDIR += ./components/libraries/usbd/class/hid/generic
UINCDIR += ./components/libraries/usbd/class/hid/kbd
UINCDIR += ./components/libraries/usbd/class/hid/mouse
UINCDIR += ./components/libraries/usbd/class/msc
UINCDIR += ./components/libraries/usbd/config
UINCDIR += ./components/libraries/util
UINCDIR += ./components/softdevice/common/softdevice_handler
UINCDIR += ./components/softdevice/s130/headers
UINCDIR += ./components/softdevice/s130/headers/nrf51
UINCDIR += ./components/toolchain
UINCDIR += ./components/toolchain/arm
UINCDIR += ./components/toolchain/gcc
UINCDIR += ./components/toolchain/CMSIS/Include
UINCDIR += ./components/toolchain/gcc
UINCDIR += ./external/segger_rtt

# List of user define
UDEFS += NRF51
UDEFS += BLE_STACK_SUPPORT_REQD
UDEFS += __HEAP_SIZE=0
UDEFS += NRF51822
UDEFS += BOARD_CUSTOM
UDEFS += BOARD_W_FILLE_MINEW_MS49SF2
UDEFS += S130
UDEFS += NRF_SD_BLE_API_VERSION=2
UDEFS += BSP_UART_SUPPORT
UDEFS += SOFTDEVICE_PRESENT
UDEFS += SWI_DISABLE0

# UASDEFS += BOARD_PCA10028
# UASDEFS += SOFTDEVICE_PRESENT
# UASDEFS += NRF51
# UASDEFS += S130
# UASDEFS += BLE_STACK_SUPPORT_REQD
# UASDEFS += SWI_DISABLE0
# UASDEFS += BSP_DEFINES_ONLY
# UASDEFS += NRF51822
# UASDEFs += NRF_SD_BLE_API_VERSION=2

UASDEFs += __HEAP_SIZE=16

# define for specific revision
UDEFS += $(SPECIFIC_REV)

# List of release define in more
RDEFS = 

# List of debug define in more
DDEFS =

# List of libraries directory
ULIBDIR =

# List of libraries
ULIBS = 

#
# End of user defines
########################################################################


########################################################################
# Start build define
#

# Binary objects directory
OBJS = .obj
# Binary ASM objects directory
DASOBJS = $(OBJS)/asm
# Binary C objects directory
DCOBJS = $(OBJS)/c

# ASM list of binary objects
ASOBJS=$(patsubst %.s,$(DASOBJS)/%.o, $(ASRC))
# C list of binary objects
COBJS=$(patsubst %.c,$(DCOBJS)/%.o, $(CSRC))

# List of include directory
INCDIR = $(patsubst %,-I%, $(UINCDIR))
# List of include library
LIBDIR = $(patsubst %,-L%, $(ULIBDIR))

# List of library
LIBS = $(patsubst %,-l%, $(ULIBS))

# List of define
_UDEFS = $(patsubst %,-D%, $(UDEFS))
_RDEFS = $(patsubst %,-D%, $(RDEFS))
_DDEFS = $(patsubst %,-D%, $(DDEFS))
_UASDEFS = $(patsubst %,-defsym %, $(UASDEFS))

#
# End build define
########################################################################

########################################################################
# Start rules section
#

all:release

# Build define for release
#.PHONY: release
# release:ASFLAGS = -mcpu=$(MCU) $(THUMB) $(RASFLAGS) $(UASFLAGS) $(_UASDEFS)
release:ASFLAGS = -mcpu=$(MCU) $(THUMB)
release:CFLAGS  = -mcpu=$(MCU) $(THUMB) $(RCFLAGS) $(UCFLAGS) $(_UDEFS) $(_RDEFS) $(INCDIR)
release:LDFLAGS = -mcpu=$(MCU) $(THUMB) $(RLDFLAGS) $(ULDFLAGS) -T$(LDSCRIPT) $(LIBDIR)
release:$(OUTPUT_DIRFILE).elf

# Build define for debug
#.PHONY: debug
debug:ASFLAGS = -mcpu=$(MCU) $(THUMB) $(DASFLAGS) $(UASFLAGS) $(_UASDEFS)
debug:CFLAGS  = -mcpu=$(MCU) $(THUMB) $(DCFLAGS) $(UCFLAGS) $(_UDEFS) $(_DDEFS) $(INCDIR)
debug:LDFLAGS = -mcpu=$(MCU) $(THUMB) $(DLDFLAGS) $(ULDFLAGS) -T$(LDSCRIPT) $(LIBDIR)
debug:$(OUTPUT_DIRFILE).elf

# Build sources to generate elf file
%.elf: $(ASOBJS) $(COBJS)
	@$(MKDIR) -p $(patsubst /%,%, $(@D))
	$(CC) -o $@ $(ASOBJS) $(COBJS) $(LDFLAGS) $(LIBS)

# Build ASM sources
.PRECIOUS: $(DASOBJS)/%.o
$(DASOBJS)/%.o: %.s
	@$(MKDIR) -p $(patsubst /%,%, $(@D))
	$(AS) $(ASFLAGS) $< -o $@

# Build C sources
.PRECIOUS: $(DCOBJS)/%.o
$(DCOBJS)/%.o: %.c
	@$(MKDIR) -p $(patsubst /%,%, $(@D))
	$(CC) $(CFLAGS) $< -c -o $@

hex:
	$(OBJCOPY) -O ihex $(OUTPUT_DIRFILE).elf $(OUTPUT_DIRFILE).hex

bin:
	$(OBJCOPY) -O binary $(OUTPUT_DIRFILE).elf $(OUTPUT_DIRFILE).bin

size:
	$(SIZE) $(OUTPUT_DIRFILE).elf
	$(SIZE) $(OUTPUT_DIRFILE).hex
# $(SIZE) $(OUTPUT_DIRFILE).bin

binsize: bin
	@du -bhs $(OUTPUT_DIRFILE).bin
	
disassemble:
	$(OBJDUMP) -hd $(OUTPUT_DIRFILE).elf > $(OUTPUT_DIRFILE).lss

itall: clean release hex bin disassemble size

itall_debug: clean debug hex bin disassemble size

# NOTE flash/debug rules here for info, untested in specific project
# Reset target
reset:
	# $(OPENOCD) -s $(OPENOCD_SCRIPT) -f $(CONFIGDIR)/openocd/init_reset.cfg -c "init; reset; shutdown"
	openocd -f $(CONFIGDIR)/openocd/init_reset.cfg -c "init; reset; shutdown"
	
	
flash: flash_program
flash_program:
	openocd -d0 -f $(CONFIGDIR)/openocd/init.cfg -c "program $(OUTPUT_DIRFILE).elf verify reset exit; shutdown"
	
# Show bank flash 0 erased or not
flash_check:
	@$(OPENOCD) -s $(OPENOCD_SCRIPT) -f $(CONFIGDIR)/openocd/init.cfg -f $(CONFIGDIR)/openocd/flash_check.cfg

# Erase flash
flash_erase:
	@$(OPENOCD) -s $(OPENOCD_SCRIPT) -f $(CONFIGDIR)/openocd/init.cfg -f $(CONFIGDIR)/openocd/flash_erase.cfg

# Erase flash and eeprom
erase_all:
	@$(OPENOCD) -s $(OPENOCD_SCRIPT) -f $(CONFIGDIR)/openocd/init.cfg -f $(CONFIGDIR)/openocd/erase_all.cfg
	
## Flash target
#st_flash_program:bin
#	st-flash --reset write $(OUTPUT_DIRFILE).bin 0x8000000
	
## dfu target
#dfu_flash_program:bin
#	dfu-util -a 0 -s 0x08000000 -D $(OUTPUT_DIRFILE).bin 0x8000000

## Erase flash
#st_flash_erase:
#	@st-flash erase
	
# Run gdb/openocd for load and debug.
gdb:debug
	$(GDB) --command=$(CONFIGDIR)/gdb/init.cfg $(OUTPUT_DIRFILE).elf
	
# Run cgdb/openocd for load and debug with color.
cgdb:debug
	cgdb -d $(GDB) --command=$(CONFIGDIR)/gdb/init.cfg $(OUTPUT_DIRFILE).elf

# Clean projet
clean:cleanOutputs
	rm -fr $(OBJS)

cleanOutputs:
ifneq ($(wildcard $(OUTPUT_DIR)*.elf),$(wildcard))
	rm -f $(OUTPUT_DIR)*.elf
endif
ifneq ($(wildcard $(OUTPUT_DIR)*.map),$(wildcard))
	rm -f $(OUTPUT_DIR)*.map
endif
ifneq ($(wildcard $(OUTPUT_DIR)*.bin),$(wildcard))
	rm -f $(OUTPUT_DIR)*.bin
endif
ifneq ($(wildcard $(OUTPUT_DIR)*.hex),$(wildcard))
	rm -f $(OUTPUT_DIR)*.hex
endif
ifneq ($(wildcard $(OUTPUT_DIR)*.lss),$(wildcard))
	rm -f $(OUTPUT_DIR)*.lss
endif
ifneq ($(wildcard $(OUTPUT_DIR)*.log),$(wildcard))
	rm -f $(OUTPUT_DIR)*.log
endif
	rm -fr $(OUTPUT_DIR)

#
# End rules section
########################################################################

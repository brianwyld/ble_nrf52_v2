########################################################################
# Start of user section
#

# Default var
SPECIFIC_REV   = 
# if not passed on command line
VER_MAJ?=2
VER_MIN?=0
# card types : 1-5 = revA-E, 10=rect. base, 11=circ. base Default is rect base
CARD_TYPE?=10

ROOT_DIR = .
SDKROOT = $(ROOT_DIR)/nrfsdk
#d:/wyres/code/BLE_V2
# Name of output file
OUTPUT_DIR     = $(ROOT_DIR)/outputs
OUTPUT_FILE    = ble_modem
OUTPUT_DIRFILE = $(OUTPUT_DIR)/$(OUTPUT_FILE)

# Directories where find config files for tools
CONFIGDIR = tools
OPENOCD = "c:/soft/openocd-0.10.0"
OPENOCD_SCRIPT = "$(OPENOCD)/share/openocd/scripts"
MKDIR = "mkdir.exe"

# GNU parameters - if gcc is on your path then leave GNU_INSTALL_ROOT empty
#C:\Program Files (x86)\GNU Tools ARM Embedded\8 2019-q3-update
GNU_INSTALL_ROOT := C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\bin
#GNU_INSTALL_ROOT := 
GNU_PREFIX := \arm-none-eabi


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
MCU = cortex-m4
# Add -mthumb for use THUMB code
#THUMB = -mthumb

#################################
# Start define flags
#

# User flags for ASM 
# UASFLAGS =

# Default flags for C 
UCFLAGS  += -Wall -std=c99
# keep every function in a separate section, this allows linker to discard unused ones
UCFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
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

# Project specific linker script file (note its in ./linker, ld uses ULIBPATH to find .ld files)
LDSCRIPT = blev2_nrf52832_xxaa_s132.ld
# derived from these ones...
#LDSCRIPT = $(SDKROOT)/config/nrf52832/armgcc/generic_gcc_nrf52.ld
#LDSCRIPT = $(SDKROOT)/components/softdevice/s132/toolchain/armgcc/armgcc_s132_nrf52832_xxaa.ld

# List ASM source files
#ASRC = ./components/toolchain/gcc/gcc_startup_nrf51.s
ASRC = $(SDKROOT)/modules/nrfx/mdk/gcc_startup_nrf52.s

# List C source files - app files last as there are most likely to fail compilation!
CSRC = $(wildcard $(ROOT_DIR)/baselibc/src/*.c)
CSRC += $(SDKROOT)/components/boards/boards.c
CSRC += $(SDKROOT)/components/ble/common/ble_advdata.c
CSRC += $(SDKROOT)/components/ble/common/ble_conn_params.c
CSRC += $(SDKROOT)/components/ble/common/ble_conn_state.c
CSRC += $(SDKROOT)/components/ble/common/ble_srv_common.c
#CSRC += $(SDKROOT)/components/ble/ble_db_discovery/ble_db_discovery.c
CSRC += $(SDKROOT)/components/ble/ble_advertising/ble_advertising.c
CSRC += $(SDKROOT)/components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c
CSRC += $(SDKROOT)/components/ble/ble_services/ble_nus/ble_nus.c
CSRC += $(SDKROOT)/components/ble/ble_services/ble_bas/ble_bas.c
CSRC += $(SDKROOT)/components/ble/ble_services/ble_dis/ble_dis.c
CSRC += $(SDKROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c
CSRC += $(SDKROOT)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c
CSRC += $(SDKROOT)/components/libraries/atomic/nrf_atomic.c
CSRC += $(SDKROOT)/components/libraries/atomic_flags/nrf_atflags.c
CSRC += $(SDKROOT)/components/libraries/button/app_button.c
CSRC += $(SDKROOT)/components/libraries/util/app_error_handler_gcc.c
CSRC += $(SDKROOT)/components/libraries/util/app_error.c
CSRC += $(SDKROOT)/components/libraries/util/app_error_weak.c
CSRC += $(SDKROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c
CSRC += $(SDKROOT)/components/libraries/fifo/app_fifo.c
CSRC += $(SDKROOT)/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c
CSRC += $(SDKROOT)/components/libraries/timer/app_timer.c
CSRC += $(SDKROOT)/components/libraries/uart/app_uart_fifo.c
CSRC += $(SDKROOT)/components/libraries/util/app_util_platform.c
CSRC += $(SDKROOT)/components/libraries/hardfault/hardfault_implementation.c
CSRC += $(SDKROOT)/components/libraries/util/nrf_assert.c
CSRC += $(SDKROOT)/components/libraries/uart/retarget.c
CSRC += $(SDKROOT)/components/libraries/fstorage/nrf_fstorage.c
CSRC += $(SDKROOT)/components/libraries/log/src/nrf_log_backend_serial.c
CSRC += $(SDKROOT)/components/libraries/log/src/nrf_log_frontend.c
CSRC += $(SDKROOT)/components/softdevice/common/nrf_sdh.c
CSRC += $(SDKROOT)/components/softdevice/common/nrf_sdh_ble.c
CSRC += $(SDKROOT)/components/softdevice/common/nrf_sdh_soc.c
CSRC += $(SDKROOT)/modules/nrfx/drivers/src/nrfx_clock.c
CSRC += $(SDKROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c
CSRC += $(SDKROOT)/modules/nrfx/drivers/src/nrfx_uarte.c
CSRC += $(SDKROOT)/modules/nrfx/mdk/system_nrf52.c
CSRC += $(SDKROOT)/integration/nrfx/legacy/nrf_drv_uart.c
CSRC += $(wildcard $(ROOT_DIR)/src/*.c)

# List of directories to include
UINCDIR = $(ROOT_DIR)/includes
UINCDIR += $(ROOT_DIR)/baselibc/include
UINCDIR += $(ROOT_DIR)/baselibc/src/templates
UINCDIR += $(SDKROOT)/config/nrf52832/config
UINCDIR += $(SDKROOT)/components
UINCDIR += $(SDKROOT)/components/ble/ble_advertising
#UINCDIR += $(SDKROOT)/components/ble/ble_db_discovery
#UINCDIR += $(SDKROOT)/components/ble/ble_dtm
UINCDIR += $(SDKROOT)/components/ble/ble_link_ctx_manager
#UINCDIR += $(SDKROOT)/components/ble/ble_racp
UINCDIR += $(SDKROOT)/components/ble/ble_services/ble_bas
UINCDIR += $(SDKROOT)/components/ble/ble_services/ble_dfu
UINCDIR += $(SDKROOT)/components/ble/ble_services/ble_nus
UINCDIR += $(SDKROOT)/components/ble/ble_services/ble_dis
UINCDIR += $(SDKROOT)/components/ble/common
UINCDIR += $(SDKROOT)/components/ble/nrf_ble_gatt
#UINCDIR += $(SDKROOT)/components/ble/nrf_ble_gq
UINCDIR += $(SDKROOT)/components/ble/nrf_ble_qwr
#UINCDIR += $(SDKROOT)/components/ble/nrf_ble_scan
#UINCDIR += $(SDKROOT)/components/ble/peer_manager
UINCDIR += $(SDKROOT)/components/boards
UINCDIR += $(SDKROOT)/components/libraries/atomic
UINCDIR += $(SDKROOT)/components/libraries/atomic_flags
UINCDIR += $(SDKROOT)/components/libraries/balloc
UINCDIR += $(SDKROOT)/components/libraries/button
UINCDIR += $(SDKROOT)/components/libraries/crc16
UINCDIR += $(SDKROOT)/components/libraries/crc32
UINCDIR += $(SDKROOT)/components/libraries/delay
UINCDIR += $(SDKROOT)/components/libraries/experimental_section_vars
UINCDIR += $(SDKROOT)/components/libraries/fds
UINCDIR += $(SDKROOT)/components/libraries/fifo
UINCDIR += $(SDKROOT)/components/libraries/fstorage
UINCDIR += $(SDKROOT)/components/libraries/hardfault
UINCDIR += $(SDKROOT)/components/libraries/led_softblink
UINCDIR += $(SDKROOT)/components/libraries/log
UINCDIR += $(SDKROOT)/components/libraries/log/src
UINCDIR += $(SDKROOT)/components/libraries/mem_manager
UINCDIR += $(SDKROOT)/components/libraries/memobj
UINCDIR += $(SDKROOT)/components/libraries/mutex
UINCDIR += $(SDKROOT)/components/libraries/pwr_mgmt
UINCDIR += $(SDKROOT)/components/libraries/queue
UINCDIR += $(SDKROOT)/components/libraries/strerror
UINCDIR += $(SDKROOT)/components/libraries/timer
UINCDIR += $(SDKROOT)/components/libraries/uart
UINCDIR += $(SDKROOT)/components/libraries/util
UINCDIR += $(SDKROOT)/components/softdevice/common
UINCDIR += $(SDKROOT)/components/softdevice/s132/headers
UINCDIR += $(SDKROOT)/components/softdevice/s132/headers/nrf52
UINCDIR += $(SDKROOT)/components/toolchain
UINCDIR += $(SDKROOT)/components/toolchain/arm
UINCDIR += $(SDKROOT)/components/toolchain/gcc
UINCDIR += $(SDKROOT)/components/toolchain/cmsis/include
UINCDIR += $(SDKROOT)/components/toolchain/gcc
UINCDIR += $(SDKROOT)/modules/nrfx
UINCDIR += $(SDKROOT)/modules/nrfx/drivers
UINCDIR += $(SDKROOT)/modules/nrfx/drivers/include
UINCDIR += $(SDKROOT)/modules/nrfx/hal
UINCDIR += $(SDKROOT)/modules/nrfx/mdk
UINCDIR += $(SDKROOT)/integration/nrfx
UINCDIR += $(SDKROOT)/integration/nrfx/legacy

# List of user define
UDEFS = USE_APP_CONFIG
UDEFS += NRF52832_XXAA
UDEFS += S132
UDEFS += BOARD_CUSTOM
UDEFS += BOARD_W_BLE_MINEW_MS50SFA2
UDEFS += BSP_UART_SUPPORT
UDEFS += SWI_DISABLE0
UDEFS += BLE_STACK_SUPPORT_REQD
UDEFS += SOFTDEVICE_PRESENT
UDEFS += NRF_SD_BLE_API_VERSION=7

UASDEFS = NRF52832_XXAA=1
UASDEFS += BOARD_CUSTOM=1
UASDEFS += USE_APP_CONFIG=1
# Note defining __STARTUP_CONFIG means setup will use include/startup_config.h 
UASDEFS += __STARTUP_CONFIG=1

# define for specific revision and set fw major/minor values
UDEFS += $(SPECIFIC_REV) 
UDEFS += FW_MAJOR=$(VER_MAJ)
UDEFS += FW_MINOR=$(VER_MIN)
UDEFS += CARD_TYPE=$(CARD_TYPE)

# List of release define in more
RDEFS = 

# List of debug define in more
DDEFS =

# List of libraries directory (including to be able to find nrf_common.ld file included from linker script)
#ULIBDIR = $(SDKROOT)/modules/nrfx/mdk
ULIBDIR = $(ROOT_DIR)/linker

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
_UASDEFS = $(patsubst %,--defsym %, $(UASDEFS))

#
# End build define
########################################################################

########################################################################
# Start rules section
#

all:release

# Build define for release
#.PHONY: release
release:ASFLAGS = -mcpu=$(MCU) $(THUMB) $(RASFLAGS) $(UASFLAGS) $(_UASDEFS) $(INCDIR)
#release:ASFLAGS = -mcpu=$(MCU) $(THUMB)
release:CFLAGS  = -mcpu=$(MCU) $(THUMB) $(RCFLAGS) $(UCFLAGS) $(_UDEFS) $(_RDEFS) $(INCDIR)
release:LDFLAGS = -mcpu=$(MCU) $(THUMB) $(RLDFLAGS) $(ULDFLAGS) -T$(LDSCRIPT) $(LIBDIR)
release:$(OUTPUT_DIRFILE).elf

# Build define for debug
#.PHONY: debug
debug:ASFLAGS = -mcpu=$(MCU) $(THUMB) $(DASFLAGS) $(UASFLAGS) $(_UASDEFS) $(INCDIR)
debug:CFLAGS  = -mcpu=$(MCU) $(THUMB) $(DCFLAGS) $(UCFLAGS) $(_UDEFS) $(_DDEFS) $(INCDIR)
debug:LDFLAGS = -mcpu=$(MCU) $(THUMB) $(DLDFLAGS) $(ULDFLAGS) -T$(LDSCRIPT) $(LIBDIR)
debug:$(OUTPUT_DIRFILE).elf

# Build sources to generate elf file
%.elf: cobjs aobjs #$(COBJS) $(ASOBJS) 
	@$(MKDIR) -p $(patsubst /%,%, $(@D))
	$(CC) -o $@ $(ASOBJS) $(COBJS) $(LDFLAGS) $(LIBS)

cobjs: $(COBJS)
	$(info compiled C files)
	
aobjs: $(ASOBJS)
	$(info assembled S files)
	
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

# incremental dev, just rebuild changed source files and reflash to board
dev: debug hex bin disassemble size flash

test:
	$(error $(COBJS))

# NOTE flash/debug rules here for info, untested in specific project
# Reset target
reset:
	@echo Reset board...
	nrfjprog -f nrf52 --reset
	
flash: flash_program
flashall: flash_erase flash_softdevice flash_program reset

flash_program:
	@echo Flashing: $(OUTPUT_DIRFILE).hex
	nrfjprog -f nrf52 --program $(OUTPUT_DIRFILE).hex --sectorerase --verify
	nrfjprog -f nrf52 --reset

flash_boot:
	# THIS DOESNT WORK - NO RUN AFTER
	@echo Flashing: hexs\bootloader.hex
	nrfjprog -f nrf52 --program $(ROOT_DIR)\hexs\bootloader.hex --sectorerase --verify

flash_softdevice:
	@echo Flashing: s132_nrf52_7.2.0_softdevice.hex from sdk
	nrfjprog -f nrf52 --program $(SDKROOT)\components\softdevice\s132\hex\s132_nrf52_7.2.0_softdevice.hex --sectorerase --verify

# Erase flash
flash_erase:
	@echo Erasing flash...
	nrfjprog -f nrf52 --eraseall
	
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

# Description

This project is made to compile and use the BLE iBeacon + scanner Wyres firmware using GCC toolchain.
Hardware is a Minew MS50SFA2 module using a nRF52832-QFAA Nordic Semi bluetooth chip.

It uses the Nordic SDK/SoftDevice system:
 - SDK : nRF5_SDK_17.1.0
 - Softdevice : S132 v7.2 

# Boards
Several boards use this module:
 - W_BLE_BASE : a rectangular board with 2 grove style connectors, one for I2C and one for UART, plus a 2x6 header
 - W_BLE_ROND : a round PCB designed to have a coin cell holder on one side, and 2 grove connectors on the other, plus 2 LEDs
 - W_FILLE : Wyres specific daughtercard designed to fit on top of a W_BASE_V2 Lora card

Board files are provided to map the GPIOs for each board type (in include/)

Default : Wyres W_BLE_BASE rev A board 
```
Pinouts: (module pin/nrf pin designation / QFN48 pin)
CN1 (grove) - UART - P1 is square
 P1 - GND
 P2 - VCC
 P3 - UART TX (pin 14 / P0.17 / 20)
 P4 - UART RX (pin 15 / P0.18 / 21)

CN3 (grove) - I2C  - P1 is square
 P1 - GND
 P2 - VCC
 P3 - SDA (pin 12 / P0.11 / 14)
 P4 - SCL (pin 13 / P0.12 / 15)

CN2 (1x5 square pads array) : SWD - P1 is leftmost pad with CN2 at bottom
 P1 - SWDIO (26)
 P2 - SWCLK (25)
 P3 - 3.3V
 P4 - GND
 P5 - nRST (P0.21 / 24)
 
CN4 : IO - P1 top left viewed from above with BLE antenna upwards
 P1 - GPIO (pin 2 / P0.27 / 39)		P2 - GND
 P3 - GPIO (pin 3 / P0.28 /	40)	    P4 - GPIO (pin 18 / P0.21 / 24)
 P5 - GPIO (pin 4 / P0.29 / 41)		P6 - GPIO (pin 5 / P0.30 / 42)
 P7 - GPIO (pin 6 / P0.10 /	12)	    P8 - GPIO (pin 7 / P0.09 / 11)
 P9 - GPIO (pin 8 / P0.03 / 5 )		P10- GPIO (pin 9 / P0.04 / 6)
```

The application maps the GPIOs as follows for this board in board_ms50SFA2_w_ble.h:
- P0.27 - CN4/p1   LED2 (HOST_WAKEUP)
- P0.28 - CN4/p3   LED1 (LED)
- P0.21 - CN4/p4   U_WAKEUP (RESET)
- P0.29 - CN4/p5   PDM (audio in)
- P0.30 - CN4/p6   I2S (audio out)
- P0.10 - CN4/p7   I2S (audio out)
- P0.09 - CN4/p8   I2S (audio out)
- P0.03 - CN4/p9    BUT1
- P0.04 - CN4/p10   BUT2

# Setup

## Sources
All the source files, header files and linker scripts are located in the project.

## Nordic SDK
Current version is included in the gitlab project. To update:
- download from https://www.nordicsemi.com/Products/Development-software/nrf5-sdk/download
- Unzip and copy to src tree in ./nrfsdk/

Also download the nrf tools:
- https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download#infotabs

In particular nrfjprog.exe is good to flash device using a jlink probe.

## Make
Get a version appropriate for your system and ensure it is in the PATH.

## GCC for arm:
- install it somewhere, point to it from Makefile GNU_INSTALL_ROOT

## Debug probe : 
J-LINK
- Flashing the device is easiest with a JLINK probe wired to a 5 way 'clothes peg' pogopin tool that clips to CN2.
- jlink commander is used, update path in makefile to point to it or have in PATH

ST-LinkV2
- OpenOCD can be used with an ST-Linkv2 to flash the nrf52, installed in c:/soft/openocd-0.10.0 (otherwise go change flash_w_stlink.bat)
- ST-Link utility can be used to flash hexes using the ST-LinkV2 probe

# Building

You have to set your gcc path in makefile IFF it is not in your PATH eg:
GNU_INSTALL_ROOT := C:\Program Files (x86)\GNU Tools ARM Embedded\8 2019-q3-update 

- "make clean" to clean all previous outputs.
- "make itall VER_MAJ=X VER_MIN=Y" to build everything for production (no debug) with version reported as X.Y (note that must be >2.0 to work with base card)
- "make itall_debug" to build everything for debug (optionally with version, 2.0 will be used as default)
- "dev" to do incremental build (only changed C files) and auto flash

By default the 'CARD_TYPE' parameter is 10, for the rectangular breakout board (continuously powered, uart controlled by VCC on the UART grove connector).

If using a daughter card revC/D/E version, you MUST set the 'CARD_TYPE' parameter to 3 (revC), 4 (revD), 5 (revE) by adding "CARD_TYPE=X" to the make line.

The build results are placed in ./outputs, named as ble_modem.X.

# Nordic SDK/Softdevice config

The critical config file is ./includes/app_config.h - this overrides selected config defines (from the generic ./nrfsdk/config/NRF52832/sdk_config.h) to enable/disable blocks. In particular note that just because a C file is in the makefile list to be compiled/linked, its contents may only be compiled if 'enabled' via a define... 

The SDK includes (in nrfsdk/components) top level ble code, softdevice ble protocol api and libraries which aid in using these... The drivers are in modules, and have been migrated to a 'nrfx' structure, but the libraires are still using the 'legacy' structre (nrf_drv_X), so there is a "integration/nrfx" directory that maps old use to new use.... finding the right files to include and the magic defines is fun...

# Flashing
## flashing with J-LINK

With the JLINK attached to the CN2, the makefile provides targets to :
- erase and flash everything
- flash just the S132 softdevice hex (from the nrfsdk install)
- flash just the application hex (from outputs/ble_modem.hex)

Alternatively you can use the flash_w_jlink.bat script to flash everything.

## flashing with ST-LINK (untested)

Attach ST-Link to the target and run "flash_w_stlink.bat" (in script) with arguments of the hex file (path from project root, without the suffix), and default major/minor  as 4 digit hex values

Example : for flashing the most recent build file ble_modem.hex for the preidentified target 0x0002002e type :
```
flash_w_stlink.bat outputs/ble_modem 0002 002e
```

# Debugging
## debugging with J-link (untested)

The debug process on vscode is based on launch.json settings file and uses just jlink commander (no gdb)
Have a look on launch.json to proper use it (jlink path, executable, etc).

After that you can launch debugging as usual on vscode.

## debugging with st-link (untested)

The debug process on vscode is based on launch.json settings file and uses just openocd (no gdb)
Have a look on launch.json to proper use it (openocd path, executable, etc).

After that you can launch debugging as usual on vscode.


# Warnings
## Known issues
- >1  button causes the app_button init to fail

## TODO
- add means to build initial flash config as a hex file
- add build time init of modules depending on the build-time hardware config
    - direct button input
    - direct LED output
    - I2C setup
    - MCP23008 ioexpander driver
    - button input via MCP
    - LED output via MCP
    - OLED driver + AT commands to display messages
    - I2S output driver + AT commands for audio play
    - PDM input driver + AT commands for audio record
- add button -> wakeup to UART host feature
- add AT command to get reason for wakeup by host (button, BLE wakeup, ibeacon scan done etc)
- add AT commands for control of LED(s)
- low power operation tuning
- proper BLE config via GATT chars instead of AT commands...

## FLASH layout

See the ld files in ./linker which define flash and RAM usage by the softdevice.

Generally we have:
 - 0x0000-0x1000 : init code (set by softdevice)
 - 0x1000-0x26000 : softdevice hex
 - 0x26000 - 0x77A00 : application hex
 - 0x77A00 - 0x78000 : application NVM flash page
 - 0x78000 - 0x80000 : bootloader hex

This is set up in the .ld file, which then exports symbols for each block.

## RAM alignment to softdevice

nRF52832-QFAA : 	Code size : 512 kB, Page size : 1024 byte, Nb pages : 512, 
					RAM size : 64kB, Block size = 8 kB


With softdevice enabled in your project, you must be strictly aligned to start your application RAM use after the SoftDevice RAM :

```
APP_RAM_BASE address(minimum required value) = 0x20000000 + SoftDevice RAM consumption (Minimum: 0x20001668)
```
See the ld files in ./linker which define flash and RAM usage by the softdevice

Note: Central and Peripheral operation consume a lot of ram : see components/softdevice/common/softdevice_handle/app_ram_base.h

## Static Memory consumption 

After a build success, a brief look at .map allows to know the amount of RAM used by any function :
For example : 
```
.bss COMMON         0x2000396c      0x168 .obj/c/./src/ibs_scan.o
```
shows memory consumption of ibs_scan.c. (due to declaration of ibs_scan_result_t ibs_scan_results[IBS_SCAN_LIST_LENGTH]; )

Note that this project uses baselibc project from github to reduce RAM and flash consumption!

Further investigation are needed to reduce the global RAM memory consumption to avoid this kind of returned error :
```
arm-none-eabi/bin/ld.exe: region RAM overflowed with stack collect2.exe: error: ld returned 1 exit status
```

TBD : max config on nRF52832_xxAA

## DFU BLE secure bootloader
Can be built from secure_bootloader in nrfsdk\examples\dfu
```
> cd nrfsdk\examples\dfu\secure_bootloader\pca10040_s132_ble\armgcc
 - generate private key
> nrfutil nrf5sdk-tools keys generate toto
 - generate public key as C file for build:
> nrfutil nrf5sdk-tools keys display --key pk --format code toto > ../../../dfu_public_key.c
 - build
> make nrf52832_xxaa_s132
```

Copy the resulting _build\nrf52832_xxaa_s132.hex to .\hexs as bootloader.hex

Copy the private key somewhere private to use when generating signed DFU zips (using nrfutil)

Load to the device using the flash-bootloader target in the makefile.

WARNING : THIS DOES NOT WORK AS EXPECTED, DEVICE REFUSES TO START APPLICATION! TO BE DEBUGGED...










BLE_V2_nrf52

Target 
MCU: nRF52832-QFAA : 64Mhz M4
	Flash : 512kB
	RAM : 64kB
Board : Wyres W_BLE_BASE rev A board using Minew MS50SFA2 module

Pinouts:
CN1 (grove) - UART - P1 is square
 P1 - UART RX (pin 15 / P0.18)
 P2 - UART TX (pin 14 / P0.17)
 P3 - VCC
 P4 - GND

CN3 (grove) - I2C  - P1 is square
 P1 - SCL (pin 13 / P0.12)
 P2 - SDA (pin 12 / P0.11)
 P3 - VCC
 P4 - GND

CN2 : SWD - P1 is leftmost pad with CN2 at bottom
 P1 - SWDIO
 P2 - SWCLK
 P3 - 3.3V
 P4 - GND
 P5 - nRST
 
CN4 : IO - P1 top left viewed from above with BLE antenna upwards
 P1 - P0.27 (pin 2)		P2 - GND
 P3 - P0.28 (pin 3)		P4 - P0.21 (pin 18)
 P5 - P0.29 (pin 4)		P6 - P0.30 (pin 5)
 P7 - P0.10 (pin 6)		P8 - P0.09 (pin 7)
 P9 - P0.03 (pin 8)		P10- P0.04 (pin 9)

1. Tools
From https://www.nordicsemi.com/Products/Development-software/nrf5-sdk/download
Nordic SoftDevice S132 version 7.2.0
Nordic nrf52 SDK version 17.1.0
Unzip and copy to src tree in ./nrfsdk/

Make : 
GCC for arm:
installed somewhere, point to it from Makefile GNU_INSTALL_ROOT

2. Build DFU BLE secure bootloader
Build from secure_bootloader in nrfsdk\examples\dfu
> cd nrfsdk\examples\dfu\secure_bootloader\pca10040_s132_ble\armgcc
 - generate private key
> nrfutil nrf5sdk-tools keys generate toto
 - generate public key as C file for build:
> nrfutil nrf5sdk-tools keys display --key pk --format code toto > ../../../dfu_public_key.c
 - build
> make nrf52832_xxaa_s132

Copy the resulting _build\nrf52832_xxaa_s132.hex to .\hexs as bootloader.hex
Copy the private key somewhere private to use when generating signed DFU zips (using nrfutil)

3. Device setup
Flash it with both softdevice and bootloader
Load app over the DFU or via flashing


2. migration
linker - flash usage must use correct ld file to set offset
sd_ble_enable()
 - use to find out SD ram usage
 - config now done via sd_ble_cfg_set() calls (p31)

JLINK Commander
> connect

LoadFile c:/work/dev/BLE_V2_nrf52/outputs/ble_modem.hex
LoadFile c:/work/dev/BLE_V2_nrf52/nrfsdk/components/softdevice/s132/hex/s132_nrf52_7.2.0_softdevice.hex

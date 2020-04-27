# Description

This project is made to compile and use the BLE iBeacon + scanner Wyres firmware under GCC toolchain.
The whole toolchain is located in the directory but the last commit of the makefile references pre-installed gcc toolchain.

# Use

All the source files, toolchain, includes files and linker scripts are located in the project.


## makefile

You have to set your gcc path in makefile :
GNU_INSTALL_ROOT := C:\Program Files (x86)\GNU Tools ARM Embedded\8 2019-q3-update 

Run "make cleanOutputs" to clean all previous outputs.
Run "make itall" to build everything. hex and elf files are copied in C:/dev/BLE_MODEM for debugging purposes.

## flashing with ST-LINK

Attach ST-Link to the target and run "flash_w_stlink.bat" with "app_hex_file.hex" as first argument and "major_minor_identifier_in_hex" as second argument.

Example : for flashing ble_modem.hex for the preidentified target 0x0002002e type :
```
flash_w_stlink.bat ble_modem.hex 0002002e
```

## debugging with st-link

The debug process on vscode is based on launch.json settings file.
Have a look on launch.json to proper use it (openocd path, gdb, executable, etc).

After that you can launch debugging as usual on vscode.

## debugging with j-link

The debug process on vscode is based on launch.json settings file.
Have a look on launch.json to proper use it (openocd path, gdb, executable, etc).

If your target has to be powered by jlink, be sure to entering "power on" or "power on perm" with jlink.exe before.  

```
jlink.exe > power on
jlink.exe > exit
```

After that you can launch debugging as usual on vscode.


# Warning

The make command works and build the hex file BUT the firmware does not seem to work properly as intended (iBeacon mode does not work for example, not seen by Wyres Android app). The problem is maybe located in the linker script, further investigations needed.

#Known issues

## RAM alignment

nRF51822-QFAA : 	Code size : 256 kB, Page size : 1024 byte, Nb pages : 256, 
					RAM size : 16kB, Block 0 = 8 kB, Block 1 = 8 kB 

As described in softdevice doc : S130_SDS_v2.0.pdf : 15.1 Memory resource map and usage	(see in this repo at /documentation)

With softdevice enabled in your project, you must be stricly aligned with SoftDevice RAM :

```
APP_RAM_BASE address(minimum required value) = 0x20000000 + SoftDevice RAM consumption (Minimum: 0x200013C8)
```

Then your linker script describes RAM section as :

```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 0x1889F
  RAM (xrw) : ORIGIN = 0x20002000, LENGTH = 0x2000
  USER_CFG(r!x) : ORIGIN = 0x338A0, LENGTH = 0x1000
  
}
```

## User defined symbols and variables

For this project the heap memory allocation is not necessary and may be set to zero.
Heap memory is configured in gcc_nrf51_startup.s according to user definition.
The problem is located when the assembler arm-none-eabi-as is called with its arguments : 
```
-defsym __HEAP_SIZE=16
```
seems to be ignored...
To fix this : gcc_nrf51_startup.s has been modified to set zero for heap size by default.

```
#if defined(__STARTUP_CONFIG)
    .equ Heap_Size, __STARTUP_CONFIG_HEAP_SIZE
#elif defined(__HEAP_SIZE)
    .equ Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0
#endif
```

## Static Memory consumption 

After a build success, a brief look at .map allows to know the amount of RAM used by any function :
For example : 
```
.bss COMMON         0x2000396c      0x168 .obj/c/./src/ibs_scan.o
```
shows memory consumption of ibs_scan.c. (due to declaration of ibs_scan_result_t ibs_scan_results[IBS_SCAN_LIST_LENGTH]; )

libc memory use also a big amount of RAM...

Further investigation are needed to reduce the global RAM memory consumption to avoid this kind of returned error :
```
arm-none-eabi/bin/ld.exe: region RAM overflowed with stack collect2.exe: error: ld returned 1 exit status
```















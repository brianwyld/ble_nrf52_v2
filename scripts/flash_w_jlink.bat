@echo off
setlocal
:: app file name in %1, major %2, minor %3
set HEX=%1
if [%HEX%]==[] set HEX=outputs\ble_modem.hex
::set arg2=%2
::set major_minor=%arg2:~6,2%%arg2:~4,2%%arg2:~2,2%%arg2:~0,2%
set JLINK="c:\Program Files\SEGGER\JLink\JLink.exe"
for /f %%i in ('cd') do set ROOT=%%i

:: beware of space at end of certain commands, jlink doesn't like it (for device name for example)
echo device nRF52832_xxAA> cmd.jlink
echo si SWD>> cmd.jlink
echo speed 4000>> cmd.jlink
echo connect>> cmd.jlink
echo h>> cmd.jlink
:: bootloader hex copied from building dfu examples eg nrfsdk\examples\dfu\secure_bootloader\pca10040_s132_ble\_build\nrf52832_xxaa_s132.hex
:: DO NOT USE FOR NOW - STOPS EXECUTION?
:: echo LoadFile %ROOT%\hexs\bootloader.hex >> cmd.jlink
echo LoadFile %ROOT%\nrfsdk\components\softdevice\s132\hex\s132_nrf52_7.2.0_softdevice.hex >> cmd.jlink
echo LoadFile %ROOT%\%HEX% >> cmd.jlink
echo reset>> cmd.jlink
echo q>> cmd.jlink
:: write softdevice, app hex, cfg area, then reset
%JLINK% -commandfile cmd.jlink

endlocal
exit /b
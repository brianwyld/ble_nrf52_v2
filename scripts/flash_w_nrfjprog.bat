@echo off
setlocal
:: app file name in %1, major %2, minor %3
set HEX=%1
if [%HEX%]==[] set HEX=outputs\ble_modem.hex
::set arg2=%2
::set major_minor=%arg2:~6,2%%arg2:~4,2%%arg2:~2,2%%arg2:~0,2%
for /f %%i in ('cd') do set ROOT=%%i

nrfjprog -f nrf52 --eraseall
:: bootloader hex copied from building dfu example eg nrfsdk\examples\dfu\secure_bootloader\pca10040_s132_ble\_build\nrf52832_xxaa_s132.hex
:: DO NOT USE FOR NOW - STOPS EXECUTION?
:: nrfjprog -f nrf52 --program %ROOT%\hexs\bootloader.hex --sectorerase --verify
nrfjprog -f nrf52 --program %ROOT%\nrfsdk\components\softdevice\s132\hex\s132_nrf52_7.2.0_softdevice.hex --sectorerase --verify
nrfjprog -f nrf52 --program %ROOT%\%HEX% --verify --sectorerase 
nrfjprog -f nrf52 --reset
endlocal
exit /b
/* Linker script to configure memory regions. */
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 0x1889F
  RAM (xrw) : ORIGIN = 0x20002000, LENGTH = 0x2000
/* RAM (rwx) :  ORIGIN = 0x200013c8, LENGTH = 0x2c38 */
  USER_CFG(r!x) : ORIGIN = 0x338A0, LENGTH = 0x1000
  
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(.pwr_mgmt_data))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "./linker/nrf5x_common.ld"
/*INCLUDE "./components/toolchain/gcc/gcc_nrf51_common.ld"*/
/* Linker script to configure memory regions. */
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /* flash is 0x40000 size (256kb) for nRF51822 */
  /* With softdevice, our app starts at 0x1B000 (see S130 SoftDevice spec v2.0 Memory resource Map and Usage) */
  /* We leave 4K (0x1000) for our config function at the end */
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 0x40000-0x1b000-0x1000     
  FLASH_CFG(rw!x) : ORIGIN = 0x40000-0x1000, LENGTH = 0x1000

  /* RAM starts at 0x20000000 and is size 0x4000 (16kb) */
  /* our RAM area is just after softdevice area , up to the end */
  /* Softdevice S130 requires 0x1288+'configurable resources': min is 0x13C8 if using minimal resources (no info on what that might mean)
  /* so we start at this value, for a length of (0x4000-0x13C8) ie 0x2c38*/
  RAM (rwx) :  ORIGIN = 0x200013c8, LENGTH = 0x4000-0x13C8  
  /* Note we set the heap to size 0 */
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

/* allow code to know where the space for config is, and its size by exporting as linker variables */
PROVIDE(__FLASH_CONFIG_BASE_ADDR = ORIGIN(FLASH_CFG));
PROVIDE(__FLASH_CONFIG_SZ = LENGTH(FLASH_CFG));

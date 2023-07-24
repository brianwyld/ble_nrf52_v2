/* Linker script to configure memory regions. */
/* This is specific for this project (based on generic_gcc_nrf52.ld from nrfsdk/config/nrf52832/armgcc) */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /* flash starts at 0x00000 and is 0x80000 size (512kb) for nRF52832xxaa */
  /* Softdevice S132 hex is loaded at 0x01000 and uses up to 0x25000 (SD_FLASH_SIZE in nrf_sdm.h), so our app starts at 0x26000 (APP_CODE_BASE) and has 0x50000 to play with */
  /* app must live at APP_CODE_BASE?
  /* bootloader lives after the application at 0x78000 so we only use up to that point
  /* We leave 1K (0x400) for our application specific config store at the end of the flash after app area. */
  
  FLASH (rx) : ORIGIN = 0x26000, LENGTH = 0x78000-0x26000-0x400
  FLASH_CFG (rw!x) : ORIGIN = 0x78000-0x400, LENGTH = 0x400

  /* RAM starts at 0x20000000 and is size 0x10000 (64kb) */
  /* Softdevice S132 starts at the beginning and uses X amount where X depends on the number of connections/DB etc defined */
  /* our RAM area is just after softdevice area , up to the end */
  /* Softdevice S132 requires 0x1668? (looking at armgcc_s132_nrf52832_xxaa.ld from nrfsdk/components/softdevice/s132/toolchain/armgcc) */
  /* plus extra for N connections... - examples/blinky/S132 has 0x5968 */ 
  /* 1 Central + 1 periph + 1 UUID -> ? S132 v7 seems to have some method of working it out */
  /* round up to 0x5a00 */
  RAM (rwx) :  ORIGIN = 0x20005a00, LENGTH = 0xa600
}


/* allow app code to know where the space for config is, and its size by exporting as linker variables */
PROVIDE(__FLASH_CONFIG_BASE_ADDR = ORIGIN(FLASH_CFG));
PROVIDE(__FLASH_CONFIG_SZ = LENGTH(FLASH_CFG));

SECTIONS
{
}

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM
  .cli_sorted_cmd_ptrs :
  {
    PROVIDE(__start_cli_sorted_cmd_ptrs = .);
    KEEP(*(.cli_sorted_cmd_ptrs))
    PROVIDE(__stop_cli_sorted_cmd_ptrs = .);
  } > RAM
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .sdh_ant_observers :
  {
    PROVIDE(__start_sdh_ant_observers = .);
    KEEP(*(SORT(.sdh_ant_observers*)))
    PROVIDE(__stop_sdh_ant_observers = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH
  .sdh_req_observers :
  {
    PROVIDE(__start_sdh_req_observers = .);
    KEEP(*(SORT(.sdh_req_observers*)))
    PROVIDE(__stop_sdh_req_observers = .);
  } > FLASH
  .sdh_state_observers :
  {
    PROVIDE(__start_sdh_state_observers = .);
    KEEP(*(SORT(.sdh_state_observers*)))
    PROVIDE(__stop_sdh_state_observers = .);
  } > FLASH
  .sdh_stack_observers :
  {
    PROVIDE(__start_sdh_stack_observers = .);
    KEEP(*(SORT(.sdh_stack_observers*)))
    PROVIDE(__stop_sdh_stack_observers = .);
  } > FLASH
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
    .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH
    .nrf_queue :
  {
    PROVIDE(__start_nrf_queue = .);
    KEEP(*(.nrf_queue))
    PROVIDE(__stop_nrf_queue = .);
  } > FLASH
    .cli_command :
  {
    PROVIDE(__start_cli_command = .);
    KEEP(*(.cli_command))
    PROVIDE(__stop_cli_command = .);
  } > FLASH
  .crypto_data :
  {
    PROVIDE(__start_crypto_data = .);
    KEEP(*(SORT(.crypto_data*)))
    PROVIDE(__stop_crypto_data = .);
  } > FLASH
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH

} INSERT AFTER .text


INCLUDE "nrf_common.ld"



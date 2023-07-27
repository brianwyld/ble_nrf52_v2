#ifndef H_APP_CONFIG_H
#define H_APP_CONFIG_H

#define NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION 499
#define NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION 65535

#define APP_UART_ENABLED 1
#define APP_FIFO_ENABLED 1
#define APP_TIMER_ENABLED 1
#define BLE_NUS_ENABLED 1
#define BLE_BAS_ENABLED 1
#define BLE_DIS_ENABLED 1
#define BLE_QWR_ENABLED 1
#define BLE_ADVERTISING_ENABLED 1
#define NRF_BLE_GATT_ENABLED 1
#define NRF_BLE_QWR_ENABLED 1
#define NRF_PWR_MGMT_ENABLED 1
#define BUTTON_ENABLED 1
#define NRF_SDH_ENABLED 1
#define NRF_SDH_SOC_ENABLED 1
#define NRF_SDH_BLE_ENABLED 1
#define NRF_SECTION_ITER_ENABLED 1
#define NRF_BLE_CONN_PARAMS_ENABLED 1

// BLE stackj config
// <i> Requested BLE GAP data length to be negotiated.
#define NRF_SDH_BLE_GAP_DATA_LENGTH 251
// <o> NRF_SDH_BLE_PERIPHERAL_LINK_COUNT - Maximum number of peripheral links. 
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
// <o> NRF_SDH_BLE_CENTRAL_LINK_COUNT - Maximum number of central links. 
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 1
// <o> NRF_SDH_BLE_TOTAL_LINK_COUNT - Total link count. 
// <i> Maximum number of total concurrent connections using the default configuration.
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 2
// <o> NRF_SDH_BLE_GAP_EVENT_LENGTH - GAP event length. 
#define NRF_SDH_BLE_GAP_EVENT_LENGTH 6
// <o> NRF_SDH_BLE_GATT_MAX_MTU_SIZE - Static maximum MTU size. 
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 247
// <o> NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE - Attribute Table size in bytes. The size must be a multiple of 4. 
#define NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE 1408
// <o> NRF_SDH_BLE_VS_UUID_COUNT - The number of vendor-specific UUIDs. 
#define NRF_SDH_BLE_VS_UUID_COUNT 2
// <q> NRF_SDH_BLE_SERVICE_CHANGED  - Include the Service Changed characteristic in the Attribute Table.
#define NRF_SDH_BLE_SERVICE_CHANGED 0

#undef NRF_LOG_ENABLED
#undef NRF_LOG_BACKEND_UART_ENABLED

// enabling anything from nrfx drivers via legacy layer requires no NRFX_ prefix
#define GPIOTE_ENABLED 1
#define UART_ENABLED 1
#define UART0_ENABLED 1
#define UART_EASY_DMA_SUPPORT  1
#define UART0_CONFIG_USE_EASY_DMA 1
#define UART_LEGACY_SUPPORT 0
#define APP_UART_DRIVER_INSTANCE 0
/*
wutils
`app_error_handler

from main
`NRF_LOG_DEFAULT_BACKENDS_INIT

bsp_minew_nrf52
`__FLASH_CONFIG_SZ
`__FLASH_CONFIG_BASE_ADDR

from app_uart_fifo:
`nrf_drv_uart_use_easy_dma
`nrf_drv_uart_init
`nrfx_uarte_tx_in_progress
`nrfx_uarte_uninit
`nrfx_uarte_rx
*/
#endif /* H_APP_CONFIG_H */
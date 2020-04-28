/*
 * Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_button.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "boards.h"
#include "bsp_config.h"
#include "ble.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "bsp_btn_ble.h"
#include "ble_gap.h"
#include "ble_nus.h"
#include "ble_hci.h"
#include "ble_gatts.h"
#include "softdevice_handler.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nrf_delay.h"

#include "timer_watchdog.h"

#include "wutils.h"
#include "bsp_minew_nrf51.h"
#include "configmgr.h"
#include "comm_uart.h"
#include "comm_ble.h"

#include "ibs_scan.h"

#ifndef FW_MAJOR
#define FW_MAJOR (2)
#endif
#ifndef FW_MINOR 
#define FW_MINOR (0)
#endif
#define IS_SRVC_CHANGED_CHARACT_PRESENT 0   /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              1   /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1   /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE    GATT_MTU_SIZE_DEFAULT           /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */
 
#define DEVICE_NAME_BASE             "w"                                       /**< Name of device when for connection beacons. Will be included in the advertising data. */
#define DEVICE_NAME_LEN             16      // w_0000_0000

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */
 
#define APP_ADV_INTERVAL_SLOW           64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       180                                         /**< The advertising timeout (in units of seconds). */
 
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */ 
 
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */
 
#define DEAD_BEEF 0xDEADBEEF                                    /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN      /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_TIMER_PRESCALER     0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 2                               /**< Size of timer operation queues. */

#define MIN_CONNECTION_INTERVAL MSEC_TO_UNITS(20, UNIT_1_25_MS) /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL MSEC_TO_UNITS(75, UNIT_1_25_MS) /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY           0                               /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT     MSEC_TO_UNITS(4000, UNIT_10_MS) /**< Determines supervision time-out in units of 10 millisecond. */
 
//WBeacon
#define APP_BEACON_INFO_LENGTH          0x17                                    /**< Total length of information advertised by the Beacon. */
#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                   /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */

static struct app_context {
    //UART Variables
    uint16_t                m_conn_handle;    /**< Handle of the current GAP connection. */
    ble_uuid_t              m_adv_uuids[2];  /**< Universally unique service identifier. */
    ble_db_discovery_t      m_ble_db_discovery;             /**< Instance of database discovery module. Must be passed to all db_discovert API calls */
    // config
    bool bleConnectable;        // is it allowed to connect via GAP for config or uart pass-thru?
    bool isIBeaconning;         // should we be acting as ibeacon just now?
    //iBeacon config
    uint16_t advertisingInterval_ms; // Default advertising interval 100 ms
    uint16_t major_value;
    uint16_t minor_value;
    uint8_t beacon_uuid_tab[UUID128_SIZE];
    uint16_t company_id; // apple 0x0059 -> Nordic
    int8_t txPowerLevel; // Default -4dBm
    uint8_t extra_value;    // usually related to tx power
    uint8_t device_type;        // for ibeacon?
    // config for connectable adverts
    char nameAdv[DEVICE_NAME_LEN];
    bool isPasswordOK;              // Indicates if password has been validated or not
    uint8_t passwordTab[PASSWORD_LEN]; // Password
    uint8_t masterPasswordTab[PASSWORD_LEN]; // Password
    uint8_t adv_data_length;
    ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
    uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH]; 
    bool flashWriteReq;
    bool flashBusy;
    bool resetRequested;
} _ctx = {
    .m_conn_handle = BLE_CONN_HANDLE_INVALID, 
    .m_adv_uuids = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}},
    .bleConnectable=true,
    .isIBeaconning=false,
    .advertisingInterval_ms = 300, 
    .txPowerLevel = RADIO_TXPOWER_TXPOWER_Neg20dBm,  
    .major_value = 0xFFFF,
    .minor_value = 0xFFFF,
    .beacon_uuid_tab = {0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, 0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0},
    .nameAdv = "W_FFFF_FFFF", 
    .adv_data_length = 0x15,
    .company_id = 0x004C, 
    .extra_value = 0xC3,
    .device_type = 0x02,
    .passwordTab = {'0', '0', '0', '0'},
    .masterPasswordTab = {'6', '0', '6', '7'},
};

static void sys_evt_handler(uint32_t evt);
static uint32_t advertising_check_start();
static void configInit();
static void configUpdateRequest();
static void configWrite();
static uint8_t makeAdvPowerLevel( void );

//APP_TIMER_DEF(m_deinit_uart_delay); 

#if 0
volatile char debug;
    uint8_t var;
#endif 

/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

 
/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
 
    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);
 
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}
 
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
        
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
 
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
 
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
 
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}
 
 

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    comm_ble_init();
}
 
/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
 
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(_ctx.m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
        err_code = advertising_check_start();
        configUpdateRequest();      // As was connected, params may have changed
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
 
    memset(&cp_init, 0, sizeof(cp_init));
 
    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
 
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;
 
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
        break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
        break;
        default:
        break;
    }
}
  
/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;
 
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            _ctx.m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        break; // BLE_GAP_EVT_CONNECTED
 
        case BLE_GAP_EVT_DISCONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            _ctx.m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code = advertising_check_start();
            configUpdateRequest();      // As was connected, params may have changed
            APP_ERROR_CHECK(err_code);
        break; // BLE_GAP_EVT_DISCONNECTED
        
        case BLE_GAP_EVT_ADV_REPORT: {
            ble_gap_evt_adv_report_t *p_adv_report = &p_gap_evt->params.adv_report;
            ibs_handle_advert(p_adv_report);
        break; // BLE_GAP_EVT_ADV_REPORT
        }
        case BLE_GAP_EVT_TIMEOUT:
            err_code = advertising_check_start();
            configUpdateRequest();      // As was connected, params may have changed
            APP_ERROR_CHECK(err_code);
            
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
            {
                // If scan still active, restart it
                if (ibs_is_scan_active())
                {
                    ibs_scan_restart();
                }
            }
        break;
            
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(_ctx.m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
        break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST
 
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(_ctx.m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
        break; // BLE_GATTS_EVT_SYS_ATTR_MISSING
 
        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            err_code = advertising_check_start();
            configUpdateRequest();      // As was connected, params may have changed
            APP_ERROR_CHECK(err_code);
        break; // BLE_GATTC_EVT_TIMEOUT
 
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            err_code = advertising_check_start();
            configUpdateRequest();      // As was connected, params may have changed
            APP_ERROR_CHECK(err_code);
        break; // BLE_GATTS_EVT_TIMEOUT
 
        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
        break; // BLE_EVT_USER_MEM_REQUEST
 
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;
 
            req = p_ble_evt->evt.gatts_evt.params.authorize_request;
 
            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
 
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
 
 
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        }
        break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
 
#if (NRF_SD_BLE_API_VERSION == 3)
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
        break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif
 
        default:
            // No implementation needed.
        break;
    }
}

/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    comm_ble_disconnect();
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *          been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    // Dispatch to ble stack services if we want to be connected
    if (_ctx.bleConnectable)
    {
        ble_conn_params_on_ble_evt(p_ble_evt);
        comm_ble_dispatch(p_ble_evt);           // For NUS service (we are slave)
//        comm_ble_c_dispatch(p_ble_evt);       // When discovering peers for NUS client connections
    }
//    bsp_btn_ble_on_ble_evt(p_ble_evt);
    // If we are scanning then make db of remote guys be up to date
    if (ibs_is_scan_active())
    {
        ble_db_discovery_on_ble_evt(&_ctx.m_ble_db_discovery, p_ble_evt);
    }
    on_ble_evt(p_ble_evt);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,
                                PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by bsp
 */
/*
static void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
        break;

        case BSP_EVENT_DISCONNECT:
            comm_ble_disconnect();
            advertising_check_start();
            configUpdateRequest();
        break;
        case BSP_EVENT_WHITELIST_OFF:
            advertising_check_start();
        break;
            
        default:
        break;
    }
}
*/
/** @brief Function for initializing the Database Discovery Module.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/** Config handling
 */
// Get config from NVM into _ctx
static void configInit() {
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_MAJOR, &_ctx.major_value, sizeof(uint16_t));
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_MINOR, &_ctx.minor_value, sizeof(uint16_t));
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_PERIOD_MS, &_ctx.advertisingInterval_ms, sizeof(uint16_t));
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_TXPOWER, &_ctx.txPowerLevel, sizeof(int8_t));
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_EXTRA, &_ctx.extra_value, sizeof(uint8_t));
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_COMPANYID, &_ctx.company_id, sizeof(uint16_t));
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_NAME, &_ctx.nameAdv, DEVICE_NAME_LEN);
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_UUID, &_ctx.beacon_uuid_tab, UUID128_SIZE);
    CFMgr_getOrAddElement(CFG_UTIL_KEY_BLE_IBEACON_PASS, &_ctx.passwordTab, PASSWORD_LEN);
}
// Request update of NVM with new config
static void configUpdateRequest() {
    // Set flag to do it in main loop (for ble timing reasons)
    _ctx.flashWriteReq = true;
}
static void configWrite() {
    _ctx.flashBusy=true;
    // Wrtie each value using configmgr (who checks if it has changed)
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_MAJOR, &_ctx.major_value, sizeof(uint16_t));
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_MINOR, &_ctx.minor_value, sizeof(uint16_t));
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_PERIOD_MS, &_ctx.advertisingInterval_ms, sizeof(uint16_t));
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_TXPOWER, &_ctx.txPowerLevel, sizeof(int8_t));
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_EXTRA, &_ctx.extra_value, sizeof(uint8_t));
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_COMPANYID, &_ctx.company_id, sizeof(uint16_t));
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_NAME, &_ctx.nameAdv, DEVICE_NAME_LEN);
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_UUID, &_ctx.beacon_uuid_tab, UUID128_SIZE);
    CFMgr_setElement(CFG_UTIL_KEY_BLE_IBEACON_PASS, &_ctx.passwordTab, PASSWORD_LEN);
//    _ctx.flashBusy=false;     // reset in system callback apparently
}
/*!
* Set And Get methods
*/
int cfg_getFWMajor() {
    return FW_MAJOR;
}
int cfg_getFWMinor() {
    return FW_MINOR;
}

void cfg_setUUID(uint8_t* value)
{
    memcpy(_ctx.beacon_uuid_tab, value, UUID128_SIZE);
    configUpdateRequest();
}
 
uint8_t* cfg_getUUID( void )
{
    return _ctx.beacon_uuid_tab;
}
void cfg_setADV_IND(uint32_t value)
{
    _ctx.advertisingInterval_ms = value;
    configUpdateRequest();
}
 
uint16_t cfg_getADV_IND( void )
{
    return _ctx.advertisingInterval_ms;
}
 
void cfg_setTXPOWER_Level(int8_t level)
{
    _ctx.txPowerLevel = level;
    configUpdateRequest();
}
 
int8_t cfg_getTXPOWER_Level( void )
{
    return _ctx.txPowerLevel;
}
 
void cfg_setMinor_Value(uint16_t value)
{
    _ctx.minor_value = value;
    configUpdateRequest();
}
 
uint16_t cfg_getMinor_Value( void )
{
    return _ctx.minor_value;
}
 
void cfg_setMajor_Value(uint16_t value)
{
    _ctx.major_value = value;
    configUpdateRequest();
}
 
uint16_t cfg_getMajor_Value( void )
{
    return _ctx.major_value;
}
 
void cfg_resetPasswordOk()
{
   _ctx.isPasswordOK = false;
}
 
bool cfg_checkPassword( char* given )
{
    for(int i=0;i<PASSWORD_LEN;i++) {
        if (_ctx.passwordTab[i]!=given[i]) {
            _ctx.isPasswordOK = false;
            return false;
        }
    }
    _ctx.isPasswordOK = true;
    return true;
}
bool cfg_setPassword(char* oldp, char* newp) 
{
    if (cfg_checkPassword(oldp)) {
        for(int i=0;i<PASSWORD_LEN;i++) {
            _ctx.passwordTab[i]=newp[i];
        }
        configUpdateRequest();
        return true;
    }
    return false;
} 
void cfg_setCompanyID(uint16_t value)
{
    _ctx.company_id = value;
    configUpdateRequest();
}
 
uint16_t cfg_getCompanyID( void )
{
    return _ctx.company_id;
}
 
void cfg_setExtra_Value(uint8_t extra) {
    _ctx.extra_value = extra;
    configUpdateRequest();
}
uint8_t cfg_getExtra_Value() {
    return makeAdvPowerLevel();
}

// Return the 'measured power @ 1m' field value for the ibeacon
static uint8_t makeAdvPowerLevel( void )
{
    // If explciitly set (eg via AT cmd) use value 
    if (_ctx.extra_value!=0) {
        return _ctx.extra_value;
    }
    // Otherwise we use value based on tx power and battery
    uint8_t result_adv = 0;
    float battery = 0.0;
    
    // Get voltage
    hal_bsp_adc_init();
    battery = (float)hal_bsp_adc_read_u16();
    hal_bsp_adc_disable();
    battery = (battery * 3.6) / 1024.0;
 
    if(battery > 3.2)
    {
        result_adv = 0x18;
    }
    else if((battery <= 3.2) && (battery > 3.0))
    {
        result_adv = 0x10;
    }
    else if((battery <= 3.0) && (battery > 2.9))
    {
        result_adv = 0x08;
    }
    else if(battery <= 2.9)
    {
        result_adv = 0x00;
    }
    else
    {
        result_adv = 0x00;
    }
    
    // add tx power to the byte
    switch(cfg_getTXPOWER_Level())
    {
        case (int8_t)0x00: // RADIO_TXPOWER_TXPOWER_0dBm
            result_adv |= 0x00;
        break;
        
        case (int8_t)0xFC: // RADIO_TXPOWER_TXPOWER_Neg4dBm:
            result_adv |= 0x01;
        break;
        
        case (int8_t)0xF8: // RADIO_TXPOWER_TXPOWER_Neg8dBm:
            result_adv |= 0x02;
        break;
        
        case (int8_t)0xF4: // RADIO_TXPOWER_TXPOWER_Neg12dBm:
            result_adv |= 0x03;
        break;
        
        case (int8_t)0xF0: // RADIO_TXPOWER_TXPOWER_Neg16dBm:
            result_adv |= 0x04;
        break;
        
        case (int8_t)0xEC: // RADIO_TXPOWER_TXPOWER_Neg20dBm:
            result_adv |= 0x05;
        break;
        
        case (int8_t)0xD8: // RADIO_TXPOWER_TXPOWER_Neg30dBm:
            result_adv |= 0x06;
        break;
        
        case (int8_t)0x04UL: //RADIO_TXPOWER_TXPOWER_Pos4dBm:
            result_adv |= 0x07;
        break;
        
        default:
            result_adv |= 0x05;
        break;
    }
 
    return result_adv;
}
 
bool ibb_isBeaconning() {
    return _ctx.isIBeaconning;
}

void ibb_start() {
    _ctx.isIBeaconning=true;
    advertising_check_start();
}
void ibb_stop() {
    _ctx.isIBeaconning=false;
    advertising_check_start();
}

/** request a reset asap */
void app_reset_request() {
    _ctx.resetRequested = true;    
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advdata_t          advdata;
    ble_advdata_t          scanrsp;
    ble_adv_modes_config_t options;
    ble_advdata_manuf_data_t manuf_specific_data; 
    uint8_t flags = (BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE |
                                        BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
    memset(&advdata, 0, sizeof(advdata));
    memset(&scanrsp, 0, sizeof(scanrsp));
    memset(&options, 0, sizeof(options));

    manuf_specific_data.company_identifier = cfg_getCompanyID();

    _ctx.m_beacon_info[0] = _ctx.device_type;
    _ctx.m_beacon_info[1] = _ctx.adv_data_length;
    memcpy(&_ctx.m_beacon_info[2], &_ctx.beacon_uuid_tab[0], UUID128_SIZE);

    _ctx.m_beacon_info[18] = MSB_16(cfg_getMajor_Value());
    _ctx.m_beacon_info[19] = LSB_16(cfg_getMajor_Value());
    _ctx.m_beacon_info[20] = MSB_16(cfg_getMinor_Value());
    _ctx.m_beacon_info[21] = LSB_16(cfg_getMinor_Value());
    
    _ctx.m_beacon_info[22] = makeAdvPowerLevel();

    manuf_specific_data.data.p_data = (uint8_t *) (&_ctx.m_beacon_info[0]);
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;
 
    // Build advertising data struct to pass into @ref ble_advertising_init.
/*
    if (ble_device_type == UART_MODULE)
    {
        advdata.name_type          = BLE_ADVDATA_FULL_NAME;
        advdata.include_appearance = false;
        advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    }
    */
    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
    
    // Initialize advertising parameters (used when starting advertising).
    memset(&_ctx.m_adv_params, 0, sizeof(_ctx.m_adv_params));
    _ctx.m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    _ctx.m_adv_params.p_peer_addr = NULL;                             // Undirected advertisement.
    _ctx.m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    _ctx.m_adv_params.interval    = MSEC_TO_UNITS(cfg_getADV_IND(), UNIT_0_625_MS); //NON_CONNECTABLE_ADV_INTERVAL;
    _ctx.m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
 
    scanrsp.uuids_complete.uuid_cnt = sizeof(_ctx.m_adv_uuids) / sizeof(_ctx.m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = _ctx.m_adv_uuids;
    scanrsp.name_type = BLE_ADVDATA_FULL_NAME;
    scanrsp.p_tx_power_level = &_ctx.txPowerLevel;

    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = MSEC_TO_UNITS(cfg_getADV_IND(), UNIT_0_625_MS);
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
    options.ble_adv_slow_enabled = true;
    options.ble_adv_slow_interval = MSEC_TO_UNITS(APP_ADV_INTERVAL_SLOW, UNIT_0_625_MS);     //1285*1.6;
    options.ble_adv_slow_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
 
    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}
 
 
/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
/*
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;
 
    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);
 
    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);
 
    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
*/

static uint32_t advertising_check_start()
{
    // in all cases, setup to be ready
    ble_gap_conn_sec_mode_t sec_mode;    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    // password check has not been validated
    cfg_resetPasswordOk();
    // Update name in case major/minor changed 
    sprintf(_ctx.nameAdv, "%s_%04x_%04x", DEVICE_NAME_BASE, cfg_getMajor_Value(), cfg_getMinor_Value());
    // Change name
    sd_ble_gap_device_name_set(&sec_mode,
                               (const uint8_t *)(&_ctx.nameAdv[0]),
                                strlen(_ctx.nameAdv));
 
    // Change tx power level
    sd_ble_gap_tx_power_set(cfg_getTXPOWER_Level());
 
    advertising_init();
    conn_params_init();
    // only advertise if we want to be connectable, or we are ibeaconning, and we are NOT scanning
    if ((_ctx.bleConnectable || _ctx.isIBeaconning) && ibs_is_scan_active()==false)
    {
        return ble_advertising_start(_ctx.isIBeaconning?BLE_ADV_MODE_FAST:BLE_ADV_EVT_SLOW);
    }
    // TODO is it neccessary to stop advertising or did the init() do it?

    return 0;       // setup ok
}
 
void app_setFlashBusy() {
    _ctx.flashBusy = true;      // someone somewhere is writing to the flash
}
bool app_isFlashBusy() {
    return _ctx.flashBusy;      // they would like to know if its busy right now
}

// Callback when system events happen
static void sys_evt_handler(uint32_t evt)
{
    switch(evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
            _ctx.flashBusy = false;
        break;
        
        case NRF_EVT_FLASH_OPERATION_ERROR:
            _ctx.flashBusy = false;         // Still finished though
        break;
        
        default:
        break;
    }
}




 
/*
static void deinit_uart_timeout_handler(void * p_context)
{
    comm_uart_deinit();
    
    //
    //examples to setup TX/RX in different mode :
    //
    //nrf_gpio_cfg_default(RX_PIN_NUMBER);
    //nrf_gpio_cfg_default(TX_PIN_NUMBER);
    //
    //nrf_gpio_cfg_output(RX_PIN_NUMBER);
    //nrf_gpio_pin_set(RX_PIN_NUMBER);
    //nrf_gpio_cfg_output(TX_PIN_NUMBER);
    //nrf_gpio_pin_set(TX_PIN_NUMBER);
    //
    //nrf_gpio_cfg_input(RX_PIN_NUMBER, GPIO_PIN_CNF_PULL_Pulldown);
    //nrf_gpio_cfg_input(TX_PIN_NUMBER, GPIO_PIN_CNF_PULL_Pulldown);
}
*/

void system_init(void)
{
    uint32_t err_code;
            
    softdevice_sys_evt_handler_set(sys_evt_handler);
        
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
    
    db_discovery_init();

    wlog_init(-1);      // replace by 0 to get logs on uart
    ble_stack_init();

    comm_uart_init(); // REV B invert TX RX for board//
    comm_uart_tx((uint8_t *)"READY\r\n",7, NULL);
    
    gap_params_init();
    services_init();
    // Init adv parameters and start if required
    err_code = advertising_check_start();
    APP_ERROR_CHECK(err_code);

    //todo remove uart disabled at 10s of ibeaconning for power
    /*                    
    app_timer_create(&m_deinit_uart_delay,
                        APP_TIMER_MODE_SINGLE_SHOT,
                        deinit_uart_timeout_handler);    
        uint32_t deinit_delay = APP_TIMER_TICKS(10000, APP_TIMER_PRESCALER);//32 * 10000;          
        app_timer_start(m_deinit_uart_delay, deinit_delay, NULL);
    */
}
int main(void)
{
    configInit();              
    
    system_init();
    for (;;)
    {        
        if (_ctx.resetRequested)
        {
            NVIC_SystemReset();
        }
        if(_ctx.flashWriteReq)
        {
            configWrite();
            _ctx.flashWriteReq = false;
        }
        
        // Go in lowpower only if flash isn't busy
        if(!app_isFlashBusy())
        {
            power_manage();
        }
    }
}

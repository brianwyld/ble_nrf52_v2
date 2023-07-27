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

#include "nordic_common.h"
#include "nrf.h"

#include "app_timer.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_button.h"
#include "app_util.h"
#include "app_util_platform.h"

#include "boards.h"
#include "bsp_minew_nrf52.h"

#include "ble.h"
#include "ble_conn_params.h"
//#include "ble_db_discovery.h"
#include "ble_gap.h"
#include "ble_nus.h"
#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_bas.h"
//#include "ble_hci.h"
//#include "ble_gatts.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"
#include "nrf_pwr_mgmt.h"

#include "timer_watchdog.h"

#include "wutils.h"
#include "main.h"
#include "comm_uart.h"
#include "comm_ble.h"
#include "ble_wakehost.h"
#include "device_config.h"

#include "ibs_scan.h"


#define DEVICE_NAME                     "Smart Badge"                            /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "Infrafon"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NUM                       "CC1"                        /**< Model number. Will be passed to Device Information Service. */
// TODO get own manufacturer/org ids
#define MANUFACTURER_ID                 0x1122334455                            /**< Manufacturer ID, part of System ID. Will be passed to Device Information Service. */
#define ORG_UNIQUE_ID                   0x667788                                /**< Organizational Unique ID, part of System ID. Will be passed to Device Information Service. */

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0   /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              1   /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1   /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define NRF_BLE_MAX_MTU_SIZE    NRF_SDH_BLE_GATT_MAX_MTU_SIZE   /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#define APP_SOC_OBSERVER_PRIO           1                                           /**< Applications SOC event observer priority (must be <2) */
#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */
  
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */ 
 
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */
 
#define DEAD_BEEF 0xDEADBEEF                                    /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER     0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 2                               /**< Size of timer operation queues. */

#define MIN_CONNECTION_INTERVAL MSEC_TO_UNITS(20, UNIT_1_25_MS) /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL MSEC_TO_UNITS(75, UNIT_1_25_MS) /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY           0                               /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT     MSEC_TO_UNITS(4000, UNIT_10_MS) /**< Determines supervision time-out in units of 10 millisecond. */

#define APP_BATTERY_TIME_PERIOD APP_TIMER_TICKS(10000)           /* convert ms to ticks for apptimer */
//WBeacon
#define APP_BEACON_INFO_LENGTH          0x17                                    /**< Total length of information advertised by the Beacon. */
#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                   /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define APP_BEACON_ADV_DATA_LENGTH  (0x15)
#define APP_BEACON_ADV_DEVICE_TYPE (0x02)

// for cycling between advert packet types
#define APP_ADV_INTERVAL_MS_SLOW        500                                          /**< The advertising interval when not actively beaconing (in units of ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       1                                         /**< The advertising timeout (in units of seconds). */
typedef enum { ADVERT_TYPE_IBEACON=0, ADVERT_TYPE_TELEMETRY, ADVERT_TYPE_LAST } advtype_t;
// Our applicatio context
static struct app_context {
    //UART Variables
    uint16_t                m_conn_handle;    /**< Handle of the current GAP connection. */
    // TODO we don't need to have a gatt server db unless acting as the central to talk to periphs? Takes up a lot of memory
//    ble_db_discovery_t      m_ble_db_discovery;             /**< Instance of database discovery module. Must be passed to all db_discovert API calls */
    // config
    uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH]; 
    bool flashBusy;
    bool resetRequested;
    char advName[20];
    int8_t txPower;
    advtype_t advertType;
} _ctx = {
    .m_conn_handle = BLE_CONN_HANDLE_INVALID, 
    .advertType = ADVERT_TYPE_IBEACON,
};
/** Nordic service modules are all direct ble event observers - no need to call from our event handler
 * and their context structures are globals...
 * 
 */
// Timer ids are const pointers to static structures - so nasty
APP_TIMER_DEF(m_battery_timer_id);                                      /**< Battery timer. */
BLE_BAS_DEF(m_bas);                                                     /**< Structure used to identify the battery service. */
NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                 /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                     /**< Advertising module instance. */

static void sys_evt_handler(uint32_t evt_id, void * p_context);
static void ble_evt_dispatch(const ble_evt_t * p_ble_evt, void* p_ctx);

// Register a handler for SOC events for flash busy/free
NRF_SDH_SOC_OBSERVER(m_soc_observer, APP_SOC_OBSERVER_PRIO, sys_evt_handler, NULL);
// Register a handler for BLE events.
NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_dispatch, NULL);

static void advertising_init(ble_advertising_t* p_adv_ctx, int8_t* p_txPower);
static bool advertising_update(ble_advertising_t* p_adv_ctx, int8_t* p_txPower);
static bool advertising_check_start();
static uint8_t makeAdvPowerLevel( void );

static void button1_event(uint8_t pin_no, uint8_t button_action);
static void button2_event(uint8_t pin_no, uint8_t button_action);
static void button3_event(uint8_t pin_no, uint8_t button_action);
static app_button_cfg_t button_cfgs[BUTTONS_NUMBER] = {
    { .pin_no = BUTTON_1, .active_state=APP_BUTTON_ACTIVE_HIGH, .pull_cfg=GPIO_PIN_CNF_PULL_Pulldown, .button_handler = button1_event },
    { .pin_no = BUTTON_2, .active_state=APP_BUTTON_ACTIVE_HIGH, .pull_cfg=GPIO_PIN_CNF_PULL_Pulldown, .button_handler = button2_event },
//    { .pin_no = BSP_BUTTON_3, .active_state=APP_BUTTON_ACTIVE_HIGH, .pull_cfg=GPIO_PIN_CNF_PULL_Pulldown, .button_handler = button3_event },
};
static void battery_update_timer_cb(void * p_context);

// debug startup helpers
static void init_stage(int s);
static bool _logs = false;

//APP_TIMER_DEF(m_deinit_uart_delay); 

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


/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

typedef enum { 
    INDICATE_STARTUP_0, INDICATE_STARTUP_1, INDICATE_STARTUP_2, INDICATE_STARTUP_3, INDICATE_STARTUP_4, INDICATE_STARTUP_5, INDICATE_STARTUP_6,INDICATE_STARTUP_7, 
    INDICATE_HALT, 
    INDICATE_IDLE, INDICATE_ADVERTISING_IBEACON, INDICATE_ADVERTISING_TELEMETERY, INDICATE_CONNECTED } IND_e;

void led_indication(IND_e ind) {
    init_stage(ind);
    log_info("led : state %d",ind);
    // TODO
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    led_indication(INDICATE_IDLE);
 
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    uint32_t err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/* event from BAS service */
static void bas_evt_handler(ble_bas_t * p_bas, ble_bas_evt_t * p_evt) {
    log_info("bas event %d", p_evt->evt_type);
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
    log_info("evt:conn params evt %d", p_evt->evt_type);

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(_ctx.m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
        // We will get the BLE_GAP_EVT_DISCONNECTED event to restart advertising etc
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

 
/**@brief Function for handling events linked to our advertising.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            log_info("evt:advertsing FAST");
            break;
        case BLE_ADV_EVT_SLOW:
            log_info("evt:advertsing SLOW");
            // if not being an ibeacon we are sending just NUS adverts
            break;
        case BLE_ADV_EVT_IDLE:
            log_info("evt:advertsing IDLE");
        // BW : no sleeping on this job TODO check
//            sleep_mode_enter();
        break;
        default:
            log_info("evt:unprocessed advertsing evt %d", ble_adv_evt);
        break;
    }
}
  

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *          been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 * @param[in] p_ctx application context for this handler - not used in this case
 */
static void ble_evt_dispatch(const ble_evt_t * p_ble_evt, void* p_ctx)
{
//    log_info("ble event %d",p_ble_evt->header.evt_id);
    // Note nordic services now register their own observers so don't need to call them here

    // Now our own specific processing
    uint32_t err_code;
 
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            log_info("evt:gap connect");
            // Only allow the connection if we are configured to allow it and there is not already someone connected
            if (cfg_getConnectable()) {
                if (_ctx.m_conn_handle==BLE_CONN_HANDLE_INVALID) {
                    led_indication(INDICATE_CONNECTED);
                    // Stop advertising, we only let 1 cnx at a time
                    //sd_ble_gap_adv_stop(_ctx.adv_ctx.adv_handle);
                    _ctx.m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
                    err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, _ctx.m_conn_handle);
                    APP_ERROR_CHECK(err_code);
                    // password check has not been validated for this connection
                    cfg_resetPasswordOk();
                    log_info("evt:gap connect - connected");
                } else {
                    // Not allowed to connect
                    log_info("evt:gap connect REJECTED as connection already established");
                    err_code = sd_ble_gap_disconnect(_ctx.m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
                    APP_ERROR_CHECK(err_code);
                }
            } else {
                // Not allowed to connect
                log_info("evt:gap connect REJECTED as configured non-connectable");
                err_code = sd_ble_gap_disconnect(_ctx.m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
                APP_ERROR_CHECK(err_code);
            }
        break; // BLE_GAP_EVT_CONNECTED
 
        case BLE_GAP_EVT_DISCONNECTED:
            log_info("evt:gap disconnect");
            led_indication(INDICATE_IDLE);
            _ctx.m_conn_handle = BLE_CONN_HANDLE_INVALID;
            // Restart advertising if required
            advertising_check_start();
        break; // BLE_GAP_EVT_DISCONNECTED
        
        case BLE_GAP_EVT_ADV_REPORT: {
            log_info("evt:adv rx");
            const ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;

            // Reception of an advert - could be an ibeacon
            ibs_handle_advert(&p_gap_evt->params.adv_report);
        break; // BLE_GAP_EVT_ADV_REPORT
        }
        case BLE_GAP_EVT_TIMEOUT:
            log_info("evt:gap timeout");

            advertising_check_start();
            
            const ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;
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
            log_info("evt:gattc timeout");
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            // And we get a EVT_DISCONNECTED when its done
        break; // BLE_GATTC_EVT_TIMEOUT
 
        case BLE_GATTS_EVT_TIMEOUT:
            log_info("evt:gatts timeout");
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            // And we get a EVT_DISCONNECTED when its done
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
 
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
        break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
 
        case BLE_GAP_EVT_ADV_SET_TERMINATED:
        {
            const ble_gap_evt_adv_set_terminated_t* p_adv_term = &p_ble_evt->evt.gap_evt.params.adv_set_terminated;
            if (p_adv_term->reason==BLE_GAP_EVT_ADV_SET_TERMINATED_REASON_TIMEOUT) {
                // restart it
                log_info("advertising restart");
                advertising_check_start();
            } else {
                log_info("advertising stopped due to %d", p_adv_term->reason);
            }
            break;
        }
        default:
            // No implementation needed.
        break;
    }
}

// Callback when system events happen
static void sys_evt_handler(uint32_t evt_id, void * p_context)
{
    log_info("SOC event %d", evt_id);

    switch(evt_id)
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



/**@brief Function for handling events from the GATT library. */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((_ctx.m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        uint16_t ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        comm_ble_set_max_data_len(ble_nus_max_data_len);
        log_info("Data len is set to 0x%X(%d)", ble_nus_max_data_len, ble_nus_max_data_len);
    }
    log_info("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}

static void wakeup_host(const void * wakedata, uint32_t wd_len) {
    log_info("WAKEUP HOST request");
}

/**@brief Function for initializing the Connection Parameters module.
 * one time boot init
 */
static void conn_params_init(void)
{
    uint32_t err_code;
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

static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
 
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
 
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
 
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 * One time boot init
 */
static void services_init(void)
{   
    uint32_t err_code;
    ble_bas_init_t     bas_init;
    ble_dis_init_t     dis_init;
    ble_dis_sys_id_t   sys_id;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module
    log_info("BLE Queued Wrte (QWR) init");
    qwr_init.error_handler = nrf_qwr_error_handler;
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    log_info("BLE Battery service (BAS) init");
    memset(&bas_init, 0, sizeof(bas_init));
    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec        = SEC_OPEN;
    bas_init.bl_cccd_wr_sec   = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;

    bas_init.evt_handler          = bas_evt_handler;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    log_info("BLE Device info (DIS) init");
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.model_num_str, MODEL_NUM);

    sys_id.manufacturer_id            = MANUFACTURER_ID;
    sys_id.organizationally_unique_id = ORG_UNIQUE_ID;
    dis_init.p_sys_id                 = &sys_id;

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    log_info("BLE UART (NUS) init");
    err_code = comm_ble_init();
    APP_ERROR_CHECK(err_code);

    log_info("BLE Wakeup host service init");
    err_code = ble_wakeup_init(wakeup_host);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 * One time init at boot
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
        ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    log_info("softdevice enabled");

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);
    log_info("softdevice says app ram starts at %x", ram_start);
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);
    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
    log_info("ble enabled");

}

/* One time boot init of ble stack */
static void ble_init(void) {
    
    log_info("ble stack init");
    ble_stack_init();

    log_info("ble gap init");
    gap_params_init();

    log_info("ble gaatt init");
    gatt_init();

    log_info("ble services init");
    services_init();

    // Initialise connection parameters
    log_info("ble conn params init");
    conn_params_init();

    log_info("ble advertising params init");
    advertising_init(&m_advertising, &_ctx.txPower);
    log_info("ble stack init done");

}

/* one time system init */
static void system_init(void)
{       
    ret_code_t err_code;
    // Timers
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // LOGS   
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
    // Power mgmt
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);  

    // BSP
    // Setup gpio
    if(!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    // BW TODO WHY DOESN4T WORK WITH 2 BUTTONS????
    err_code = app_button_init(button_cfgs, 1,50);  //BUTTONS_NUMBER, 50);
    APP_ERROR_CHECK(err_code);
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_update_timer_cb);
    APP_ERROR_CHECK(err_code);

}

// Return the 'measured power @ 1m' field value for the ibeacon
static uint8_t makeAdvPowerLevel( void )
{
    // If explciitly set (eg via AT cmd) use value 
    if (cfg_getExtra_Value()!=0) {
        return cfg_getExtra_Value();
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
 
/**@brief Function for initializing the Advertising functionality params
 * call once to init the p_adv_ctx
 */
static void advertising_init(ble_advertising_t* p_adv_ctx, int8_t* p_txPower)
{
    uint32_t               err_code;
    ble_advertising_init_t ai;

    memset(&ai, 0, sizeof(ai));

    ai.evt_handler = on_adv_evt;
    ai.error_handler = NULL;
    ai.advdata.name_type             = BLE_ADVDATA_FULL_NAME;
    ai.advdata.flags                 = (BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
    // Not connectable, no uuids advertised to avoid access to NUS
    ai.srdata.uuids_complete.uuid_cnt = 0;
    ai.srdata.uuids_complete.p_uuids  = NULL;
    ai.srdata.name_type = BLE_ADVDATA_FULL_NAME;
    ai.srdata.short_name_len = 8;      // get at least id part (maj/min in hex)
    ai.srdata.p_tx_power_level = p_txPower;

    // Give both options for advert speed (the one used depends on whether isIBeaconning() is true or not)
    ai.config.ble_adv_fast_enabled  = true;
    ai.config.ble_adv_fast_interval = MSEC_TO_UNITS(cfg_getADV_IND(), UNIT_0_625_MS);
    ai.config.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS*100;
    ai.config.ble_adv_slow_enabled = true;
    ai.config.ble_adv_slow_interval = MSEC_TO_UNITS(APP_ADV_INTERVAL_MS_SLOW, UNIT_0_625_MS);     //1285*1.6;
    ai.config.ble_adv_slow_timeout  = APP_ADV_TIMEOUT_IN_SECONDS*100;
     
    err_code = ble_advertising_init(p_adv_ctx, &ai);
    APP_ERROR_CHECK(err_code);
    ble_advertising_conn_cfg_tag_set(p_adv_ctx, APP_BLE_CONN_CFG_TAG);
}
 /**@brief Function for updateing the Advertising functionality params
 * call before each start of beaconning to update from config in case it changed
 */
static bool advertising_update(ble_advertising_t* p_adv_ctx, int8_t* p_txPower)
{
    bool ret_ibeacons = false;
    uint32_t               err_code;
    ble_advdata_t advdata;
    ble_advdata_t srdata;
    ble_advdata_manuf_data_t manuf_specific_data; 
    // advertise only basic services in main advert
    ble_uuid_t m_adv_uuids[] = {
        {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
        {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE},
    };
    // and special services in the scan response 
    /* NO - too long with 2 128but vendor specific IDs!
    ble_uuid_t m_sr_uuids[] = {
        {0, 0}, // Placeholder for wakeup service UUID
        {0, 0}, // placeholder for NUS server for at commands over NUS
        // TODO add config service?
    };     
    // get wakeup service uuid
    ble_wakeup_getuuid(&m_sr_uuids[0]);
    // get NUS service uuid
    comm_ble_getuuid(&m_sr_uuids[1]);
    */

    memset(&advdata, 0, sizeof(advdata));
    memset(&srdata, 0, sizeof(srdata));

    // Build advertising data struct to pass into @ref ble_advertising_init.
    // We switch between advertising content each time so that we can mix iBeacons (if required), telemetry data and connection data.
    // This happens each time the 'adv_X_timeout' happens, which is after 1s.
    _ctx.advertType = ((_ctx.advertType+1) % ADVERT_TYPE_LAST);
    switch (_ctx.advertType) {
        case ADVERT_TYPE_IBEACON:
        {
            if (cfg_isIBeaconning()) {
                // Only make our advert look like an ibeacon if we are wanting it to be one
                manuf_specific_data.company_identifier = cfg_getCompanyID();
                // iBeacons are a BLE advert with a specific 'manufacteur specific' block, which is a TLV essentially
                _ctx.m_beacon_info[0] = APP_BEACON_ADV_DEVICE_TYPE;     // Tag - this is a ibeacon
                _ctx.m_beacon_info[1] = APP_BEACON_ADV_DATA_LENGTH;     // length of rest of block

                memcpy(&_ctx.m_beacon_info[2], cfg_getUUID(), UUID128_SIZE);
                _ctx.m_beacon_info[18] = MSB_16(cfg_getMajor_Value());
                _ctx.m_beacon_info[19] = LSB_16(cfg_getMajor_Value());
                _ctx.m_beacon_info[20] = MSB_16(cfg_getMinor_Value());
                _ctx.m_beacon_info[21] = LSB_16(cfg_getMinor_Value());    
                _ctx.m_beacon_info[22] = makeAdvPowerLevel();

                manuf_specific_data.data.p_data = (uint8_t *) (&_ctx.m_beacon_info[0]);
                manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

                advdata.name_type             = BLE_ADVDATA_NO_NAME;
                advdata.p_manuf_specific_data = &manuf_specific_data;
                advdata.flags                 = (BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
                // Not connectable, no uuids advertised to avoid access to NUS
                err_code = ble_advertising_advdata_update(p_adv_ctx, &advdata, NULL);
                APP_ERROR_CHECK(err_code);
                ret_ibeacons = true;
                break;
            } else {
                // fallthru to next type
            }
        }
        case ADVERT_TYPE_TELEMETRY:
        {
            advdata.name_type             = BLE_ADVDATA_FULL_NAME;
            // depending on if we accept connections or not, advert data is different
            if (cfg_getConnectable()) {
                advdata.flags                 = (BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE |
                                                BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
                // advertise short list of services, and say more are available (stack takes care of sending the others when someone connects to me)
                advdata.uuids_more_available.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
                advdata.uuids_more_available.p_uuids  = m_adv_uuids;
                /* no need to have SR uuid list, stack takes care of sending other declared services apparenrtly
                advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
                advdata.uuids_complete.p_uuids  = m_adv_uuids;
                srdata.uuids_more_available.uuid_cnt = sizeof(m_sr_uuids) / sizeof(m_sr_uuids[0]);
                srdata.uuids_more_available.p_uuids  = m_sr_uuids;
                */
            } else {
                // Not connectable, no more uuids advertised in advert to avoid access to NUS
                advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
                advdata.uuids_complete.p_uuids  = m_adv_uuids;
            }
            srdata.name_type = BLE_ADVDATA_FULL_NAME;
            srdata.short_name_len = 8;      // get at least id part (maj/min in hex)
            srdata.p_tx_power_level = p_txPower;
            err_code = ble_advertising_advdata_update(p_adv_ctx, &advdata, &srdata);
            APP_ERROR_CHECK(err_code);
            break;        
        }
        default:
        {
            log_warn("unknown advert type requested %d", _ctx.advertType);
            break;
        }
    } 
    return ret_ibeacons;
}
 


static bool advertising_check_start()
{
    uint32_t err_code;
    // Can't start if currently got an active BLE connection (not enough channels)
    if (_ctx.m_conn_handle!=BLE_CONN_HANDLE_INVALID) {
        return false;
    }
    // in all cases, setup to be ready otherwise the ble_advertisting_start() will fail with wrong state error
    // security - none, open adverts/scan connect
    ble_gap_conn_sec_mode_t sec_mode;    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    // password check has not been validated
    cfg_resetPasswordOk();
    // create context
    _ctx.txPower = cfg_getTXPOWER_Level();
    bool ibeacons = advertising_update(&m_advertising, &_ctx.txPower);
    // Change name
    char* advname = cfg_getAdvName();
    memset(_ctx.advName, 0, sizeof(_ctx.advName));
    strncpy(_ctx.advName, advname, sizeof(_ctx.advName)-1);
    sd_ble_gap_device_name_set(&sec_mode,
                            (const uint8_t *)_ctx.advName,
                                strlen(_ctx.advName));
    sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_HID);
//    log_info("set advName to %s",_ctx.advName);

    // Change tx power level
    // NOT RUIRED? sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_advertising.adv_handle, cfg_getTXPOWER_Level());

    // only advertise if we want to be connectable, or we are ibeaconning, and we are NOT scanning
    if ((cfg_getConnectable() || cfg_isIBeaconning())) // && ibs_is_scan_active()==false)
    {
        if (ibeacons) {
            err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
            APP_ERROR_CHECK(err_code);
            led_indication(INDICATE_ADVERTISING_IBEACON);
        } else {
            err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
            APP_ERROR_CHECK(err_code);
            led_indication(INDICATE_ADVERTISING_TELEMETERY);
        }
        log_info("adv check : connectable [%s] beaconning : %s", cfg_getAdvName(), ibeacons?"yes":"no");
        return true;
    } else {
        //idle the adverts
        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE);
        APP_ERROR_CHECK(err_code);
        led_indication(INDICATE_IDLE);
        //sd_ble_gap_adv_stop(m_advertising.adv_handle);
        log_info("adv check: stopped beaconning");
        return false;
    }
}
 


/** @brief Function for the Power manager.
 */
static void power_manage(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
    /* OLD
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
    */
}

// Control of beaconning activity
bool ibb_isBeaconning() {
    return (cfg_isIBeaconning());
}

void ibb_start() {
    cfg_setIsIBeaconning(true);
    // To change the beaconning, we have to stop anything currently running (connectable or ibeaconning)
    // TODO (the current phase will timeout in a while anyway and then we'll re-check it)
}
void ibb_stop() {
    cfg_setIsIBeaconning(false);
    // To change the beaconning, we have to stop anything currently running (connectable or ibeaconning)
    // TODO (the current phase will timeout in a while anyway and then we'll re-check it)
}

/** request a reset asap */
void app_reset_request() {
    _ctx.resetRequested = true;    
}

void app_setFlashBusy() {
    _ctx.flashBusy = true;      // someone somewhere is writing to the flash
}
void app_setFlashIdle() {
    _ctx.flashBusy = false;
}
bool app_isFlashBusy() {
    return _ctx.flashBusy;      // they would like to know if its busy right now
}
static void battery_update_timer_cb(void * p_context) {
    // TODO read battery level
    uint8_t battery_level = 99;
    ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
}

static void button1_event(uint8_t pin_no, uint8_t button_action) {
    if (button_action==APP_BUTTON_RELEASE) {
        // send event to main board
        // TODO
        bsp_board_led_invert(1);
    }
}

static void button2_event(uint8_t pin_no, uint8_t button_action) {
    if (button_action==APP_BUTTON_RELEASE) {
        // send event to main board
        // TODO
        bsp_board_led_invert(1);
    }

}
static void button3_event(uint8_t pin_no, uint8_t button_action) {
    if (button_action==APP_BUTTON_RELEASE) {
        // send event to main board
        // TODO
        bsp_board_led_invert(1);
    }

}


static void init_stage(int s) {
    switch(s) {
        case INDICATE_STARTUP_0:
            for(int i=0;i<2;i++) {
                bsp_board_led_on(0);
                nrf_delay_ms(500);
                bsp_board_led_off(0);
                nrf_delay_ms(500);
            }
            break;
        case INDICATE_STARTUP_1:
            bsp_board_led_on(0);
            bsp_board_led_off(1);
            nrf_delay_ms(1000);
            break;
        case INDICATE_STARTUP_2:
            bsp_board_led_off(0);
            bsp_board_led_on(1);
            nrf_delay_ms(1000);
            break;
        case INDICATE_STARTUP_3:
            bsp_board_led_on(0);
            bsp_board_led_on(1);
            nrf_delay_ms(1000);
            break;
        case INDICATE_STARTUP_4:
            bsp_board_led_off(0);
            bsp_board_led_off(1);
            nrf_delay_ms(200);
            for(int i=0;i<5;i++) {
                bsp_board_led_on(1);
                nrf_delay_ms(200);
                bsp_board_led_off(1);
                nrf_delay_ms(200);
            }
            break;
        case INDICATE_STARTUP_5:
            bsp_board_led_on(0);
            bsp_board_led_off(1);
            nrf_delay_ms(1000);
            break;
        case INDICATE_STARTUP_6:
            bsp_board_led_off(0);
            bsp_board_led_on(1);
            nrf_delay_ms(1000);
            break;
        case INDICATE_STARTUP_7:
            bsp_board_led_on(0);
            bsp_board_led_on(1);
            nrf_delay_ms(1000);
            break;
        case INDICATE_IDLE:
            bsp_board_led_off(0);
            bsp_board_led_off(1);
            break;
        case INDICATE_ADVERTISING_IBEACON:
            bsp_board_led_on(0);
            bsp_board_led_off(1);
            break;
        case INDICATE_ADVERTISING_TELEMETERY:
            bsp_board_led_off(0);
            bsp_board_led_on(1);
            break;
        case INDICATE_CONNECTED:
            bsp_board_led_on(0);
            bsp_board_led_on(1);
            break;
        case INDICATE_HALT:
            while(true) {
                bsp_board_led_on(0);
                bsp_board_led_on(1);
                nrf_delay_ms(100);
                bsp_board_led_off(0);
                bsp_board_led_off(1);
                nrf_delay_ms(100);
            }
            break;
        default:
            for(int i=0;i<10;i++) {
                bsp_board_led_on(0);
                bsp_board_led_on(1);
                nrf_delay_ms(200);
                bsp_board_led_off(0);
                bsp_board_led_off(1);
                nrf_delay_ms(200);
            }
            break;

    }
}

int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);
    init_stage(INDICATE_STARTUP_0);

    system_init();

    init_stage(INDICATE_STARTUP_1);

    // Init uart asap in case using for logs also
    comm_uart_init(); 

    init_stage(INDICATE_STARTUP_2);
#ifdef RELEASE_BUILD
    wlog_init(-1);      // replace by 0 to get logs on uart
#else 
    wlog_init(0);      // replace by 0 to get logs on uart - warning will disrupt remote guy if connected..
    _logs=true;
#endif

    log_error("STARTING");
    init_stage(INDICATE_STARTUP_3);

    cfg_init();              
    log_info("config/log init done");

    init_stage(INDICATE_STARTUP_4);
    
    ble_init();
    log_info("ble system init done");

    init_stage(INDICATE_STARTUP_5);

    // Tell host we are ready to rock
    comm_uart_tx((uint8_t *)"READY\r\n",7, NULL);
    // Init adv parameters and start if required
    advertising_check_start();
    log_info("advertising setup done");

    init_stage(INDICATE_STARTUP_6);

    // Start timer for battery rad updates
    app_timer_start(m_battery_timer_id, APP_BATTERY_TIME_PERIOD,NULL);

    //todo remove uart disabled at 10s of ibeaconning for power
    /*                    
    app_timer_create(&m_deinit_uart_delay,
                        APP_TIMER_MODE_SINGLE_SHOT,
                        deinit_uart_timeout_handler);    
        uint32_t deinit_delay = APP_TIMER_TICKS(10000, APP_TIMER_PRESCALER);//32 * 10000;          
        app_timer_start(m_deinit_uart_delay, deinit_delay, NULL);
    */

    log_info("waiting for commands");
    init_stage(INDICATE_IDLE);

    for (;;)
    {        
        if (_ctx.resetRequested)
        {
            NVIC_SystemReset();
        }
        // Check if flash needs updating
        cfg_writeCheck();
        // Check if UART has data to process
        comm_uart_processRX();
        // Go in lowpower only if flash isn't busy
        if(!app_isFlashBusy())
        {
            power_manage();
        }
    }
}

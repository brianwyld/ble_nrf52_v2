#ifndef IBS_SCAN_H__
#define IBS_SCAN_H__

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#define IBS_SCAN_LIST_LENGTH 500

#define IBS_SCAN_HEADER_LENGTH 9
#define IBS_SCAN_UUID_LENGTH 16
#define IBS_SCAN_MAJOR_LENGTH 2
#define IBS_SCAN_MINOR_LENGTH 2
#define IBS_SCAN_MEAS_POWER_LENGTH 1

typedef struct {
	uint16_t major;
	uint16_t minor;
	uint8_t meas_pow;
	uint8_t rssi;

} ibs_scan_result_t;


bool ibs_is_scan_active();
bool ibs_scan_start(UART_TX_FN_T dest_tx_fn);
bool ibs_scan_restart(void);
bool ibs_scan_stop(void);
void ibs_scan_set_uuid_filter(uint8_t* uuid);
void ibs_handle_advert(ble_gap_evt_adv_report_t *p_adv_report);
#endif

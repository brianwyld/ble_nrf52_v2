#ifndef IBS_SCAN_H__
#define IBS_SCAN_H__

#include <stdint.h>
#include <stdbool.h>

#define IBS_SCAN_LIST_LENGTH 16

#define IBS_SCAN_HEADER_LENGTH 9
#define IBS_SCAN_UUID_LENGTH 16
#define IBS_SCAN_MAJOR_LENGTH 2
#define IBS_SCAN_MINOR_LENGTH 2
#define IBS_SCAN_MEAS_POWER_LENGTH 1

typedef struct {
	uint8_t uuid[IBS_SCAN_UUID_LENGTH];
	uint8_t major[IBS_SCAN_MAJOR_LENGTH];
	uint8_t minor[IBS_SCAN_MINOR_LENGTH];
	uint8_t meas_pow;
	uint8_t rssi;

} ibs_scan_result_t;


bool ibs_is_scan_active();
bool ibs_scan_start(void);
bool ibs_do_scan_start(void);
bool ibs_scan_stop(void);
void ibs_scan_set_uuid_filter(uint8_t* uuid);

#endif

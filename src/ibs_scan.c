#define IBS_SCAN_C__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ibs_scan.h"
#include "ble_gap.h"
#include "app_error.h"
#include "bsp.h"
#include "ibs_ibeacon.h"
#include "app_uart.h"
#include "main.h"

/*
#define SCAN_INTERVAL           0x00BA //0x0150                          // < Determines scan interval in units of 0.625 millisecond. 0x00A0
#define SCAN_WINDOW             0x00B0 //0x0130                          // < Determines scan window in units of 0.625 millisecond. 0x0050
#define SCAN_ACTIVE             0                               // If 1, performe active scanning (scan requests).
#define SCAN_TIMEOUT            0x0001                          // < Timout when scanning. 0x0000 disables timeout.
*/
#define SCAN_SELECTIVE          0                               // < If 1, ignore unknown devices (non whitelisted).

extern uint16_t SCAN_INTERVAL;
extern uint16_t SCAN_WINDOW;
extern uint16_t SCAN_TIMEOUT;
extern uint8_t SCAN_ACTIVE;
extern uint8_t SCAN_RESULT_ACTIVE;

uint8_t ibs_scan_filter_uuid[UUID128_SIZE] = {0};

ibs_scan_result_t ibs_scan_results[IBS_SCAN_LIST_LENGTH];
int ibs_scan_result_index;

bool ibs_scan_active;
bool ibs_scan_push_mode;
bool ibs_scan_filter_uuid_active;
UART_TX_FN_T output_tx_fn;          // Where to write current scan results to

/**
 * @brief Parameters used when scanning.
 */
/*
static const ble_gap_scan_params_t m_scan_params =
{
    .active   = SCAN_ACTIVE,
    .interval = SCAN_INTERVAL,
    .window   = SCAN_WINDOW,
    .timeout  = SCAN_TIMEOUT,
    #if (NRF_SD_BLE_API_VERSION == 2)
        .selective   = 0,
        .p_whitelist = NULL,
    #endif
    #if (NRF_SD_BLE_API_VERSION == 3)
        .use_whitelist = 0,
    #endif
};
*/

static int ibs_scan_buf_list_index;
static int ibs_scan_buf_byte_index;
// 1 line of scan results Major,minor,rssi,meas_power
// MMMM,mmmm,rr,pp\n
static char ibs_scan_list_line[25];

// Predecs
static bool ibs_is_adv_pkt(uint8_t *data);
static void ibs_scan_add(uint8_t* data2, int8_t rssi);
static bool ibs_scan_uuid_match(uint8_t* uuid);

/**@brief Function to start scanning.
 */
bool ibs_scan_start() {

    if (ibs_scan_active) {
        return false;
    }
    return ibs_do_scan_start();
}
bool ibs_is_scan_active() {
    return ibs_scan_active;
}
bool ibs_do_scan_start()
{
    uint32_t err_code;
    ble_gap_scan_params_t m_scan_params;
    
    m_scan_params.interval = SCAN_INTERVAL;
    m_scan_params.window   = SCAN_WINDOW;
    m_scan_params.active = 1; //SCAN_ACTIVE;
    m_scan_params.timeout  = SCAN_TIMEOUT;
    m_scan_params.selective   = 0;
    m_scan_params.p_whitelist = NULL;

    //ibs_scan_toggle_list();
    ibs_scan_result_index = 0;
    err_code = sd_ble_gap_scan_start(&m_scan_params);
    
    if (err_code == NRF_SUCCESS)
    {
        ibs_scan_active = true;
        return true;
    }
    else
    {
        return false;
    }
}


bool ibs_scan_stop()
{
    uint32_t err_code;
    
    if (!ibs_scan_active)
    {
        return false;
    }
    
    if(ibs_scan_get_push_mode())
    {
        ibs_scan_set_push_mode(false);
    }

    err_code = sd_ble_gap_scan_stop();
    
    if (err_code == NRF_SUCCESS)
    {
        ibs_scan_active = false;
        return true;
    }
    else
    {
        return false;
    }
}

void ibs_handle_beacon(ble_gap_evt_adv_report_t * p_adv_report) 
{
    if (!ibs_scan_active)
    {
        return false;
    }
    if (ibs_is_adv_pkt(p_adv_report->data))
    {
        if (ibs_scan_uuid_match(p_adv_report->data + IBS_IBEACON_UUID_OFFSET))
        {
            ibs_scan_add(p_adv_report->data, p_adv_report->rssi);
        }
    }

}

void ibs_scan_set_uuid_filter(uint8_t* uuid)
{
    if (uuid == NULL)
    {
        ibs_scan_filter_uuid_active = false;
    }
    else
    {
        memcpy(ibs_scan_filter_uuid, uuid, UUID128_SIZE)
        ibs_scan_filter_uuid_active = true;
    }
}


bool ibs_scan_set_interval(uint8_t* interval)
{
    uint8_t digit;
    uint8_t byte = 0;
    uint16_t interv = 0;
    uint8_t loops = 0;
    int idx_src = 0;
    
    while (loops < 4)
    {
        digit = *(interval + idx_src);
            
        if (digit >= 'a')
        {
            digit -= 'a';
            digit += 10;
        }
        else if (digit >= 'A')
        {
            digit -= 'A';
            digit += 10;
        }
        else
        {
            digit -= '0';
        }

        if (idx_src % 2 == 0)
        {
            byte = (digit * 16);
        }
        else
        {
            byte += digit;
            if(loops == 1)
            {
                interv += (byte << 8);
            }
            else if(loops == 3)
            {
                 interv += byte;
            }

        }
        
        loops++;
        idx_src++;
    }
    
    SCAN_INTERVAL = interv;
    
    return true;
}

bool ibs_scan_set_window(uint8_t* window)
{
    uint8_t digit;
    uint8_t byte = 0;
    uint16_t windo = 0;
    uint8_t loops = 0;
    int idx_src = 0;
    
    while (loops < 4)
    {
        digit = *(window + idx_src);
            
        if (digit >= 'a')
        {
            digit -= 'a';
            digit += 10;
        }
        else if (digit >= 'A')
        {
            digit -= 'A';
            digit += 10;
        }
        else
        {
            digit -= '0';
        }

        if (idx_src % 2 == 0)
        {
            byte = (digit * 16);
        }
        else
        {
            byte += digit;
            if(loops == 1)
            {
                windo += (byte << 8);
               
            }
            else if(loops == 3)
            {
                 windo += byte;
            }

        }
        
        loops++;
        idx_src++;
    }
    
    SCAN_WINDOW = windo;
    
    return true;
}

bool ibs_scan_set_timeout(uint8_t* timeout)
{
    uint8_t digit;
    uint8_t byte = 0;
    uint16_t time = 0;
    uint8_t loops = 0;
    int idx_src = 0;
    
    while (loops < 4)
    {
        digit = *(timeout + idx_src);
            
        if (digit >= 'a')
        {
            digit -= 'a';
            digit += 10;
        }
        else if (digit >= 'A')
        {
            digit -= 'A';
            digit += 10;
        }
        else
        {
            digit -= '0';
        }

        if (idx_src % 2 == 0)
        {
            byte = (digit * 16);
        }
        else
        {
            byte += digit;
            if(loops == 1)
            {
                time += (byte << 8);
               
            }
            else if(loops == 3)
            {
                 time += byte;
            }

        }
        
        loops++;
        idx_src++;
    }
    
    SCAN_TIMEOUT = time;
    
    return true;
}

bool ibs_scan_set_active(uint8_t* active)
{
    uint8_t digit;

    digit = *(active);
            
    if (digit >= 'a')
    {
        digit -= 'a';
        digit += 10;
    }
    else if (digit >= 'A')
    {
        digit -= 'A';
        digit += 10;
    }
    else
    {
        digit -= '0';
    }
    
    SCAN_ACTIVE = digit;
    
    return true;
}

bool ibs_scan_set_active_flag(uint8_t* active)
{
    uint8_t digit;

    digit = *(active);
            
    if (digit >= 'a')
    {
        digit -= 'a';
        digit += 10;
    }
    else if (digit >= 'A')
    {
        digit -= 'A';
        digit += 10;
    }
    else
    {
        digit -= '0';
    }
    
    SCAN_RESULT_ACTIVE = digit;

    return true;
}


void ibs_scan_set_push_mode(bool push)
{
    ibs_scan_push_mode = push;
}

bool ibs_scan_get_push_mode()
{
    return ibs_scan_push_mode;
}

static bool ibs_scan_uuid_match(uint8_t* uuid)
{
    if (ibs_scan_filter_uuid_active)
    {
        return (memcmp((char*)uuid, (char*)ibs_scan_filter_uuid, IBS_SCAN_UUID_LENGTH) == 0);
    }
    else
    {
        return true;
    }
}

static void ibs_scan_add(uint8_t* data2, int8_t rssi)
{
    uint8_t data[22]; 
    
    // copy major, minor, measpow
    if (ibs_scan_result_index >= IBS_SCAN_LIST_LENGTH)
    {
        return;
    }
    
    memcpy(data, (data2 + 9), 22);
    
    //data += IBS_SCAN_HEADER_LENGTH;
    // Check if same major/minor/uuid not in the list already
    bool duplicate = false;

    int i;
    for (i = 0; i < ibs_scan_result_index; i++)
    {
        if (ibs_scan_results[i].major[0] == data[IBS_SCAN_UUID_LENGTH])
        {
            if (ibs_scan_results[i].major[1] == data[IBS_SCAN_UUID_LENGTH + 1])
            {
                if (ibs_scan_results[i].minor[0] == data[IBS_SCAN_UUID_LENGTH + 2])
                {
                    if (ibs_scan_results[i].minor[1] == data[IBS_SCAN_UUID_LENGTH + 3])
                    {
                        if (ibs_scan_filter_uuid_active)
                        {
                            // No need to check UUID
                            duplicate = true;
                            break;
                        }
                        else if (memcmp((uint8_t*)&ibs_scan_results[i], (uint8_t*)data, IBS_SCAN_UUID_LENGTH) == 0)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if (duplicate) {
        return;
    }
    
    if (ibs_scan_filter_uuid_active)
    {
        // No need to store UUID
        memcpy((uint8_t*)&ibs_scan_results[ibs_scan_result_index] + IBS_SCAN_UUID_LENGTH,
                    (data + IBS_SCAN_UUID_LENGTH),
                    IBS_SCAN_MAJOR_LENGTH + IBS_SCAN_MINOR_LENGTH + IBS_SCAN_MEAS_POWER_LENGTH);
    }
    else
    {
        memcpy((uint8_t*)&ibs_scan_results[ibs_scan_result_index],
                    data,
                    IBS_SCAN_UUID_LENGTH + IBS_SCAN_MAJOR_LENGTH + IBS_SCAN_MINOR_LENGTH + IBS_SCAN_MEAS_POWER_LENGTH);
    }
    
    ibs_scan_results[ibs_scan_result_index].rssi = rssi;
    
    if (ibs_scan_get_push_mode())
    {
        char line[25] = {0};
        if (ibs_scan_filter_uuid_active)
        {
            sprintf(line, "%02x%02x,%02x%02x,%02x,%02x,\r\n",
                    ibs_scan_results[ibs_scan_result_index].major[0],
                    ibs_scan_results[ibs_scan_result_index].major[1],
                    ibs_scan_results[ibs_scan_result_index].minor[0],
                    ibs_scan_results[ibs_scan_result_index].minor[1],
                    ibs_scan_results[ibs_scan_result_index].meas_pow,
                    ibs_scan_results[ibs_scan_result_index].rssi);
        }
        else
        {
            sprintf(line, "%02x%02x,%02x%02x,%2x,%2x,%02x%02x\r\n",
                    ibs_scan_results[ibs_scan_result_index].major[0],
                    ibs_scan_results[ibs_scan_result_index].major[1],
                    ibs_scan_results[ibs_scan_result_index].minor[0],
                    ibs_scan_results[ibs_scan_result_index].minor[1],
                    ibs_scan_results[ibs_scan_result_index].meas_pow,
                    ibs_scan_results[ibs_scan_result_index].rssi,
                    ibs_scan_results[ibs_scan_result_index].uuid[14],
                    ibs_scan_results[ibs_scan_result_index].uuid[15]);
        }
        ibs_at_send_uart(line);
    }
    ibs_scan_result_index++;
}
void ibs_scan_get_list_init()
{
    ibs_scan_buf_list_index = 0;
    ibs_scan_buf_byte_index = -5; // Header
}

char ibs_scan_get_list_next_byte()
{
    // Send number of beacons detected
    if (ibs_scan_buf_byte_index == -5)
    {
        ibs_scan_buf_byte_index++;
        char nb = ibs_scan_result_index;
            
        if ((nb / 16) >= 10)
        {
            return (nb / 16) - 10 + 'a';
        }
        else
        {
            return (nb / 16) + '0';
        }
    }
    else if (ibs_scan_buf_byte_index == -4)
    {
        ibs_scan_buf_byte_index++;
        char nb = ibs_scan_result_index;
        if ((nb & 0x0F) >= 10)
        {
            return (nb & 0x0F) - 10 + 'a';
        }
        else
        {
            return (nb & 0x0F) + '0';
        }
    }
    else if (ibs_scan_buf_byte_index == -3)
    {
        ibs_scan_buf_byte_index++;
        return '\r';
    }
    else if (ibs_scan_buf_byte_index == -2)
    {
        ibs_scan_buf_byte_index++;
        return '\n';
    }

    if (ibs_scan_buf_byte_index == -1 && ibs_scan_result_index == 0)
    {
        return (char)4; // EOT
    }

    // Send one line per beacon
    if (ibs_scan_buf_byte_index == -1 || ibs_scan_buf_byte_index == strlen(ibs_scan_list_line))
    {
        if (ibs_scan_buf_list_index < ibs_scan_result_index)
        {
            if (ibs_scan_filter_uuid_active)
            {
                sprintf(ibs_scan_list_line, "%02x%02x,%02x%02x,%2x,%2x,\r\n",
                        ibs_scan_results[ibs_scan_buf_list_index].major[0],
                        ibs_scan_results[ibs_scan_buf_list_index].major[1],
                        ibs_scan_results[ibs_scan_buf_list_index].minor[0],
                        ibs_scan_results[ibs_scan_buf_list_index].minor[1],
                        ibs_scan_results[ibs_scan_buf_list_index].meas_pow,
                        ibs_scan_results[ibs_scan_buf_list_index].rssi);
            }
            else
            {
                sprintf(ibs_scan_list_line, "%02x%02x,%02x%02x,%2x,%2x,%02x%02x\r\n",
                        ibs_scan_results[ibs_scan_buf_list_index].major[0],
                        ibs_scan_results[ibs_scan_buf_list_index].major[1],
                        ibs_scan_results[ibs_scan_buf_list_index].minor[0],
                        ibs_scan_results[ibs_scan_buf_list_index].minor[1],
                        ibs_scan_results[ibs_scan_buf_list_index].meas_pow,
                        ibs_scan_results[ibs_scan_buf_list_index].rssi,
                        ibs_scan_results[ibs_scan_buf_list_index].uuid[14],
                        ibs_scan_results[ibs_scan_buf_list_index].uuid[15]);
            }
            
            ibs_scan_buf_byte_index = 0;
            ibs_scan_buf_list_index++;
        }
        
        return ibs_scan_list_line[ibs_scan_buf_byte_index++];
    }
    
    if (ibs_scan_buf_byte_index < strlen(ibs_scan_list_line))
    {
        return ibs_scan_list_line[ibs_scan_buf_byte_index++];
    }
    
    return (char)4; // EOT
}

void ibs_scan_byte_to_ascii(char byte, char* ascii) {
	if ((byte / 16) >= 10) {
		ascii[0] = (byte / 16) - 10 + 'a';
	}
	else {
		ascii[0] = (byte / 16) + '0';
	}
	if ((byte & 0x0F) >= 10) {
		ascii[1] = (byte & 0x0F) - 10 + 'a';
	}
	else {
		ascii[1] = (byte & 0x0F) + '0';
	}
}

char ibs_scan_ascii_to_byte(char ascii) {
	char digit = ascii;

	if (digit >= 'a') {
		digit -= 'a';
		digit += 10;
	}
	else if (digit >= 'A') {
		digit -= 'A';
		digit += 10;
	}
	else {
		digit -= '0';
	}
	return digit;
}


static bool ibs_is_adv_pkt(uint8_t *data) {
	bool result = (data[0] == 0x02) && // 1st AD data length
				(data[1] == 0x01) && // Flags
				(data[2] == 0x06) && // flag = 0x06
				(data[3] == 0x1A) && // 2nd AD data length
				(data[4] == 0xff) && // Type : Manufacturer Specific Data
				(data[5] == 0x4C) && // Apple Company iD 0
				(data[6] == 0x00) && // Apple Company iD 1
				(data[7] == 0x02) && // Proximity Beacon Type 0
				(data[8] == 0x15); // Proximity Beacon Type 1

	return result;
}

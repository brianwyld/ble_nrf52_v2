/**
 * Copyright 2019 Wyres
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, 
 * software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
 * either express or implied. See the License for the specific 
 * language governing permissions and limitations under the License.
*/
#ifndef H_COMM_BLE_H
#define H_COMM_BLE_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t comm_ble_init(void);
void comm_ble_getuuid(ble_uuid_t* p_uuid);
void comm_ble_local_disconnected(void);
void comm_ble_remote_connected(uint16_t conn_handle);
void comm_ble_remote_disconnected(uint16_t conn_handle);
void comm_ble_set_max_data_len(uint16_t ml);
bool comm_ble_isConnected();
// Tx line. returns number of bytes not sent due to flow control. 
int comm_ble_tx(uint8_t* data, int len, UART_TX_READY_FN_T tx_ready);
void comm_ble_print_stats(PRINTF_FN_T printf, void* odev);

#ifdef __cplusplus
}
#endif

#endif  /* H_COMM_BLE_H */

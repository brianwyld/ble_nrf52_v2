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
#ifndef H_BLE_WAKEHOST_H
#define H_BLE_WAKEHOST_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@brief wakeup event handler callback type. */
typedef void (* ble_wakeup_handler_t) (const void * wakedata, uint32_t wd_len);
uint32_t ble_wakeup_init(ble_wakeup_handler_t whandler);
void ble_wakeup_getuuid(ble_uuid_t* p_uuid);
#ifdef __cplusplus
}
#endif

#endif  /* H_BLE_WAKEHOST_H */

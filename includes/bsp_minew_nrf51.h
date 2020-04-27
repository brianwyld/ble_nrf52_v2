/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef H_BSP_MINEW_NRF51_H
#define H_BSP_MINEW_NRF51_H

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// Flash related constants for nRF51 with softdevice and app
// TODO MAKE THE ADDRESSES RELATIVE TO MEMORY LAYOUT!
#define FLASH_PAGE_SZ (0x400)           // 1K page size for erasure
#define FLASH_START_ADDR (0x1b000)      // ie page 0. Defined ?
#define FLASH_CONFIG_SZ (0x1000)      // 4K
#define FLASH_CONFIG_BASE_ADDR (0x338A0) 
#define FLASH_CONFIG_BASE_PAGE ((FLASH_CONFIG_BASE_ADDR - FLASH_START_ADDR)/FLASH_PAGE_SZ)        // This is our base page in the flash layout

// Uart
#define UART_CNT (1)            // nrf only has 1 uart block..
#define UART0_RX_PIN_NUMBER (-1)
#define UART0_TX_PIN_NUMBER (-1)
#define UART0_RTS_PIN_NUMBER (-1)
#define UART0_CTS_PIN_NUMBER (-1)

#define UART_TX_BUF_SIZE        64                             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        64                             /**< UART RX buffer size. */


bool hal_bsp_nvmLock();
bool hal_bsp_nvmUnlock();
// Read and write PROM. It is caller's responsibility to nvmUnlock() before access and nvmLock() afterwards
uint8_t hal_bsp_nvmRead8(uint16_t off);
uint16_t hal_bsp_nvmRead16(uint16_t off);
bool hal_bsp_nvmRead(uint16_t off, uint8_t len, uint8_t* buf);
bool hal_bsp_nvmWrite8(uint16_t off, uint8_t v);
bool hal_bsp_nvmWrite16(uint16_t off, uint16_t v);
bool hal_bsp_nvmWrite(uint16_t off, uint8_t len, uint8_t* buf);
uint16_t hal_bsp_nvmSize();
// Uart init
bool hal_bsp_uart_init(int uartNb, int baudrate, app_uart_event_handler_t uart_event_handler);
void hal_bsp_uart_deinit(int uartNb);
// Tx line to uart. returns number of bytes not sent due to flow control
int hal_bsp_uart_tx(int uartNb, uint8_t d, int len);
// LED config
void hal_bsp_leds_init(void);
void hal_bsp_leds_deinit(void);
// buttons
void hal_bsp_buttons_init(void);
void hal_bsp_buttons_deinit(void);
// ADC
void hal_bsp_adc_init(void);
void hal_bsp_adc_disable(void);
uint16_t hal_bsp_adc_read_u16(void);
#ifdef __cplusplus
}
#endif

#endif  /* H_BSP_MINEW_NRF51_H */

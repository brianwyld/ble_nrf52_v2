/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef BOARD_MS50SFA2_W_BLE_H
#define BOARD_MS50SFA2_W_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// numbers are pin numbers on nRF52832 QFN48
// 21: P0.18 - CN1/p1   UART0
// 20: P0.17 - CN1/p2   UART0

// 15: P0.12 - CN3/p1   I2C (U0_CTS)
// 14: P0.11 - CN3/p2   I2C (U0_RTS)

// 39: P0.27 - CN4/p1   LED2 (HOST_WAKEUP)
// 40: P0.28 - CN4/p3   LED1 (LED)
// 24: P0.21 - CN4/p4   U_WAKEUP (RESET)
// 41: P0.29 - CN4/p5   PDM (audio in)
// 42: P0.30 - CN4/p6     I2S (audio out)
// 12: P0.10 - CN4/p7   I2S (audio out)
// 11: P0.09 - CN4/p8   I2S (audio out)
//  5: P0.03 - CN4/p9    BUT1
//  6: P0.04 - CN4/p10   BUT2

// LEDs definitions for Minew module MS50SFA2 on a wyres BLE board
#define LEDS_NUMBER    2

#define LED_START      27
// 40: P0.28 - CN4/p3
#define LED_1           28
// 39: P0.27 - CN4/p1
#define LED_2           27
#define LED_STOP       28

#define LEDS_ACTIVE_STATE 1

#define LEDS_LIST { LED_1, LED_2 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2

#define BUTTONS_NUMBER 2

#define BUTTON_START   3
// 5: P0.03 - CN4/p9
#define BUTTON_1       3
// 6: P0.04 - CN4/p10
#define BUTTON_2       4
#define BUTTON_STOP    4
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2 }

// 24 = P0.21 - CN4/p4
#define BSP_START_UART 21

// 21: P0.18 - CN1/p1
#define RX_PIN_NUMBER  18
// 20: P0.17 - CN1/p2
#define TX_PIN_NUMBER  17
#define CTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define RTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define HWFC           false

// I2C
#define I2C_ENABLED 1
// 15: P0.12 - CN3/p1
#define I2C_CONFIG_SCL_PIN 12
// 14: P0.11 - CN3/p2
#define I2C_CONFIG_SDA_PIN 11


// I2S
#undef I2S_ENABLED
#undef I2S_CONFIG_SCK_PIN
#undef I2S_CONFIG_LRCK_PIN
#undef I2S_CONFIG_SDOUT_PIN
#undef I2S_CONFIG_MCK_PIN
#undef I2S_CONFIG_SDIN_PIN
#define I2S_ENABLED 1
// 12: P0.10 - CN4/p7
#define I2S_CONFIG_SCK_PIN 10
// 11: P0.09 - CN4/p8
#define I2S_CONFIG_LRCK_PIN 9
// 42: P0.30 - CN4/p6
#define I2S_CONFIG_SDOUT_PIN 30
#define I2S_CONFIG_MCK_PIN 255
#define I2S_CONFIG_SDIN_PIN 255

// Low frequency clock source to be used by the SoftDevice
#ifdef S210
#define NRF_CLOCK_LFCLKSRC      NRF_CLOCK_LFCLKSRC_XTAL_20_PPM
#else
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}
#endif

#ifdef __cplusplus
}
#endif

#endif // BOARD_MS50SFA2_W_BLE_H

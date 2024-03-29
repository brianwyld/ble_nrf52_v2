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
#ifndef H_WUTILS_H
#define H_WUTILS_H

#include <inttypes.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#undef assert

enum LOGS_LEVEL { LOGS_DEBUG, LOGS_INFO, LOGS_RUN, LOGS_OFF };

// We signal production code with explicit define, not NDEBUG. This is coz NDEBUG
// removes asserts everywhere (including mynewt etc), and these asserts are generally
// not just 'test/debug' cases, but also 'badness reboot recovery' cases... 

// RELEASE_BUILD set in makefile
// Release code therefore just removes debug logs and other debug/test code surrounded by #ifndef RELEASE_BUILD/#endif
#ifdef RELEASE_BUILD 

#define log_debug(__s, ...) {do{}while(0);}
#define log_noout(__s, ...) {do{}while(0);}

#else 

#define log_debug log_debug_fn
#define log_noout log_noout_fn

#endif /* RELEASE_BUILD */

// Always have 'run' level operation : assert, warn/error and function logging
// NOTE : global assert defined in libc/assert.h, defined to call OS_CRASH() defined in os/os_fault.h which is the __assert_func()
// defined in the MCU specific os_fault.c (os/src/arch/src/cortex_m3/os_fault.c for example)
// This can be set to call the os_assert_cb() function by defined OS_ASSERT_CB: 1, which is defined in wutils.c to call the same wassert_fn() as here...
// fun times... 
#ifdef NDEBUG
// When removing assert, beware of 'unused variable' warnings. This 'void' of assert avoids that but at the cost of requiring the vars
// in the test to exist... which isn't quite C standard... note lack of ; at end...
#define assert(__e) {do{ (void)sizeof((__e)); }while(0);}
#else /* NDEBUG */ 
#define assert(__e) ((__e) ? (void)0 : wassert_fn(NULL, 0))
#undef APP_ERROR_CHECK
#define APP_ERROR_CHECK(d)  if (d!=NRF_SUCCESS) { log_error("at %d in %s, error code %d", __LINE__, __FILE__, d); }
#warning DEBUG assert/error checks enabled
#endif /* NDEBUG */
// Note: info, warn and error logs are in the image for dev and release, but may not be output depending on log level.
#define log_info log_info_fn
#define log_warn log_warn_fn
#define log_error log_error_fn

// Utility functions tried and tested
// sw test assert failure
void wassert_fn(const char* file, int lnum);
// logging
void wlog_init(int uartNb);
bool log_init_uart();
void log_deinit_uart();
bool log_check_uart_active();
void log_debug_fn(const char* sl, ...);
void log_info_fn(const char* sl, ...);
void log_warn_fn(const char* sl, ...);
void log_error_fn(const char* sl, ...);
void log_noout_fn(const char* sl, ...);
uint8_t get_log_level();
const char* get_log_level_str();
void set_log_level(uint8_t lev);
void log_mem(uint8_t* p, int len);
// More utility functions
/*
 * Write a 32 bit unsigned int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_uint32_t(uint8_t* b, uint8_t offset, uint32_t v);
/*
 * Write a 32 bit signed int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_int32_t(uint8_t* b, uint8_t offset, int32_t v); 
/*
 * Write a 16 bit unsigned int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_uint16_t(uint8_t* b, uint8_t offset, uint16_t v);
/*
 * Write a 16 bit signed int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_int16_t(uint8_t* b, uint8_t offset, int16_t v); 
/*
 * helper to read 16 bit LE from buffer (may be 0 stripped)
 */
uint16_t Util_readLE_uint16_t(const uint8_t* b, uint8_t l);
/*
 * helper to read 16 bit BE from buffer (may be 0 stripped)
 */
uint16_t Util_readBE_uint16_t(const uint8_t* b, uint8_t l);
/*
 * helper to read 32 bit LE from buffer (may be 0 stripped)
 */
uint32_t Util_readLE_uint32_t(const uint8_t* b, uint8_t l);
/*
 * Calculate simple hash from string input
 */
uint32_t Util_hashstrn(const char* s, int maxlen);

/*
 Return true if data block is not just 0's, false if it is
*/
bool Util_notAll0(const uint8_t *p, uint8_t sz);

uint8_t Util_hexdigit( char hex );
uint8_t Util_hexbyte( const char* hex );
/** convert a hex string to a byte array to avoid sscanf. Ensure 'out' is at least of size 'len'. Returns number of bytes successfully found */
int Util_scanhex(char* in, int len, uint8_t* out);

#ifdef __cplusplus
}
#endif

#endif  /* H_WUTILS_H */

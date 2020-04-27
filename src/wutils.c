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

#include <stdio.h>
#include <stdarg.h>

#include "wyres-generic/wutils.h"
#include "uart.h"

// assert gets stack to determine address of line that called it to dump in log
// also write stack into prom for reboot analysis
/**
 * replaces system assert with a more useful one
 * Note we don't generally use the file/line info as this increases the binary size too much (many strings)
 */
void wassert_fn(const char* file, int lnum) {
    // see https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
    void* assert_caller = 0;
    assert_caller = __builtin_extract_return_addr(__builtin_return_address(0));
    log_blocking_fn(1,"assert from [%8x] see APP.elf.lst", assert_caller);
}


#define MAX_LOGSZ 256
    // Default log level depending on build (can be changed by app)
#ifdef RELEASE_BUILD
static uint8_t _logLevel = LOGS_RUN;
#else /* RELEASE_BUILD */
static uint8_t _logLevel = LOGS_DEBUG;
#endif /* RELEASE_BUILD */

// Must be static buffer NOT ON STACK
static char _buf[MAX_LOGSZ];
static char _noutbuf[MAX_LOGSZ];        // for nout log
static UART* _uartDev = NULL;

// note the doout is to allow to break here in debugger and see the log, without actually accessing UART
static void do_log(char lev, const char* ls, va_list vl) {
    // protect here with a mutex?
    // level and timestamp
    uint32_t now = TMMgr_getRelTimeMS();
    sprintf(_buf, "%c%03d.%1d:", lev, (int)((now/1000)%1000), (int)((now/100)%10));
    // add log
    vsprintf(_buf+7, ls, vl);
    // add CRLF to end, checking we didn't go off end
    int len = strnlen(_buf, MAX_LOGSZ);
    if ((len+3)>=MAX_LOGSZ) {
        // oops might just have broken stuff... this is why _nooutbuf is just after _buf... gives some margin
        len=MAX_LOGSZ-3;
    }
    _buf[len]='\n';
    _buf[len+1]='\r';
    _buf[len+2]='\0';
    len+=3;
    if (_uartDev!=NULL) {
        // Ensure its open (no effect if already open)
        if (log_init_uart()==0) {
            int res = uart_write(_uartDev, (uint8_t*)_buf, len);
            if (res<0) {
                _buf[0] = '*';
                uart_write(_uartDev, (uint8_t*)_buf, 1);      // so user knows he missed something.
                // Not actually a lot we can do about this especially if its a flow control (SKT_NOSPACE) condition - ignore it
               log_noout_fn("log FAIL[%s]", _buf);      // just for debugger to watch
            }        
        } else {
            log_noout_fn("log init FAIL[%s]", _buf);      // just for debugger to watch
        }
   }
}
uint8_t get_log_level() {
    return _logLevel;
}
const char* get_log_level_str() {
    switch(get_log_level()) {
        case LOGS_RUN: {
            return "RUN";
        }
        case LOGS_INFO: {
            return "INFO";
        }
        case LOGS_DEBUG: {
            return "DEBUG";
        }
        default:{
            return "OFF";
        }
    }
}
void set_log_level(uint8_t l) {
    _logLevel = l;
}

/* configure uart device for logging */
void log_config_uart(UART* dev) {
    _uartDev = dev;
}
/* open/init UART for logging */
int log_init_uart() {
    // TODO
    return 0;
}
// close output uart for logs
void log_deinit_uart() {
    // TODO
}
/* check if logging uart is active, and deinit() it if its not (for low powerness) */
bool log_check_uart_active() {
    return false;       // not open so not active
}
void log_debug_fn(const char* ls, ...) {
    if (_logLevel<=LOGS_DEBUG) {
        va_list vl;
        va_start(vl, ls);
        do_log('D', ls, vl);
        va_end(vl);
    }
}
void log_info_fn(const char* ls, ...) {
    if (_logLevel<=LOGS_INFO) {
        va_list vl;
        va_start(vl, ls);
        do_log('I', ls, vl);
        va_end(vl);
    }
}
void log_warn_fn(const char* ls, ...) {
    if (_logLevel<=LOGS_RUN) {
        va_list vl;
        va_start(vl, ls);
        do_log('W', ls, vl);
        va_end(vl);
    }
}
void log_error_fn(const char* ls, ...) {
    if (_logLevel<=LOGS_RUN) {
        va_list vl;
        va_start(vl, ls);
        do_log('E', ls, vl);
        va_end(vl);
    }
}
void log_noout_fn(const char* ls, ...) {
    va_list vl;
    va_start(vl, ls);
    vsprintf(_noutbuf, ls, vl);
    // watch _noutbuf to see the log
    int l = strlen(_noutbuf);
    _noutbuf[l++] = '\n';
    _noutbuf[l++] = '\r';
    _noutbuf[l++] = '\0';
    va_end(vl);
}

// Called from sysinit once uarts etc are up
void wlog_init(void) {
    bool res = true;
    // no console
    // If specific device for logging, create its wskt driver driver
    UART* u=uart_line_comm_create("uart2", 115200);
    assert(res);
    // And tell logging to use it whenrequired
    log_config_uart(u);  
    res=(log_init_uart()==0);    // kick it off
    assert(res);
}

// More utility functions
/*
 * Write a 32 bit unsigned int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_uint32_t(uint8_t* b, uint8_t offset, uint32_t v) {
    b[offset] = (uint8_t)(v & 0xFF);
    b[offset+1] = (uint8_t)((v & 0xFF00) >> 8);
    b[offset+2] = (uint8_t)((v & 0xFF0000) >> 16);
    b[offset+3] = (uint8_t)((v & 0xFF000000) >> 24);
}

/*
 * Write a 32 bit signed int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_int32_t(uint8_t* b, uint8_t offset, int32_t v) {
    b[offset] = (uint8_t)(v & 0xFF);
    b[offset+1] = (uint8_t)((v & 0xFF00) >> 8);
    b[offset+2] = (uint8_t)((v & 0xFF0000) >> 16);
    b[offset+3] = (uint8_t)((v & 0xFF000000) >> 24);
}
/*
 * Write a 16 bit unsigned int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_uint16_t(uint8_t* b, uint8_t offset, uint16_t v) {
    b[offset] = (uint8_t)(v & 0xFF);
    b[offset+1] = (uint8_t)((v & 0xFF00) >> 8);
}
/*
 * Write a 16 bit signed int as LE format into a buffer at specified offset. 
 */
void Util_writeLE_int16_t(uint8_t* b, uint8_t offset, int16_t v) {
    b[offset] = (uint8_t)(v & 0xFF);
    b[offset+1] = (uint8_t)((v & 0xFF00) >> 8);
}

/*
 * helper to read 32 bit LE from buffer (may be 0 stripped)
 */
uint32_t Util_readLE_uint32_t(uint8_t* b, uint8_t l) {
    uint32_t ret = 0;
    for(int i=0;i<4;i++) {
        if (b!=NULL && i<l) {
            ret += (b[i] << 8*i);
        } // else 0
    }
    return ret;
}
/*
 * helper to read 16 bit LE from buffer (may be 0 stripped)
 */
uint16_t Util_readLE_uint16_t(uint8_t* b, uint8_t l) {
    uint16_t ret = 0;
    for(int i=0;i<2;i++) {
        if (b!=NULL && i<l) {
            ret += (b[i] << 8*i);
        } // else 0
    }
    return ret;
}

/* 
Return true if data block is not just 0's, false if it is
*/
bool Util_notAll0(const uint8_t *p, uint8_t sz)
{
    assert(p != NULL);
    for (int i = 0; i < sz; i++)
    {
        if (*(p + i) != 0x00)
        {
            return true;
        }
    }
    return false;
}
/*
 * Calculate simple hash from string input
 */
#define MULTIPLIER (37)
uint32_t Util_hashstrn(const char* s, int maxlen) {
    int i=0;
    uint32_t h = 0;
    /* cast s to unsigned const char * */
    /* this ensures that elements of s will be treated as having values >= 0 */
    unsigned const char *us = (unsigned const char *) s;

    while(us[i] != '\0' && i<maxlen) {
        h = h * MULTIPLIER + us[i];
        i++;
    } 

    return h;
}

/* uart_comm.c : handle the UART used for the communication with host ie AT commands
 */
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <stdio.h>

#include "app_uart.h"

#include "wutils.h"

#include "main.h"
#include "bsp_minew_nrf51.h"
#include "at_process.h"

#define COMM_UART_NB    (0)
#define COMM_UART_BAUDRATE  (UART_BAUDRATE_BAUDRATE_Baud115200)     // MUST USE THE CONSTANT NOT A SIMPLE VALUE
#define MAX_RX_LINE (60)

static struct {
    int uartNb;
    bool isOpen;
    uint8_t rx_buf[MAX_RX_LINE+2];      // wriggle space for the \0
    uint8_t rx_index;
    UART_TX_READY_FN_T tx_ready_fn;     // in case caller wants to be told
} _ctx;

// predecs
static void uart_event_handler(app_uart_evt_t * p_event);

/**@brief Function for initializing the UART.
 */
bool comm_uart_init(void) {
    _ctx.uartNb = COMM_UART_NB;
    _ctx.isOpen = hal_bsp_uart_init(_ctx.uartNb, COMM_UART_BAUDRATE, uart_event_handler);
    return _ctx.isOpen;
}

void comm_uart_deinit(void) {
    _ctx.isOpen = false;
    hal_bsp_uart_deinit(_ctx.uartNb);
}
int comm_uart_tx(uint8_t* data, int len, UART_TX_READY_FN_T tx_ready) {
    if (_ctx.isOpen) {
        _ctx.tx_ready_fn = tx_ready;        // in case of..
        // check if disconnecting 
        if (data!=NULL)  {
            return (hal_bsp_uart_tx(_ctx.uartNb, data, len));
        } else {
            // can't disconnect from uart... but let them know
            return (hal_bsp_uart_tx(_ctx.uartNb, "AT+DISC\r\n", 10));
        }
        return 0;       // ok mate
    } else {
        // soz
        return -1;
    }
}

// handler for reinit timer
static void reinit_uart_timeout_handler(void * p_context)
{
    if (_ctx.isOpen==false) {
        comm_uart_init();
    } // else ignore
}

// Timer to try a reinit in case UART is playing up...
static void start_reopen_timer() {
    /*
    app_timer_create(&_ctx->m_reinit_uart_delay,
                        APP_TIMER_MODE_SINGLE_SHOT,
                        reinit_uart_timeout_handler);    
        uint32_t reinit_delay = APP_TIMER_TICKS(10000, APP_TIMER_PRESCALER);//32 * 10000;          
        app_timer_start(_ctx->m_reinit_uart_delay, reinit_delay, NULL);
        */
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
static void uart_event_handler(app_uart_evt_t * p_event)
{    
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
        {
            // Read all the bytes we can 
            while(app_uart_get(&_ctx.rx_buf[_ctx.rx_index])==NRF_SUCCESS) 
            {
                // End of line? or buiffer full?
                if( (_ctx.rx_buf[_ctx.rx_index] == '\r') || (_ctx.rx_buf[_ctx.rx_index] == '\n') || (_ctx.rx_index >= (MAX_RX_LINE-2)) )
                {
                    _ctx.rx_buf[_ctx.rx_index] = '\n';  // make sure its got a LF on end
                    _ctx.rx_index++;
                    _ctx.rx_buf[_ctx.rx_index] = 0; // null terminate the data in buffer (not overwriting the \r or \n though)
                    // And process
                    at_process_input((char*)(&_ctx.rx_buf[0]), &comm_uart_tx);
                    // reset our line buffer to start
                    _ctx.rx_index = 0;
                    // Continue for rest of data in input
                } else {
                    _ctx.rx_index++;
                }
            }
        }
        break;
 
        case APP_UART_COMMUNICATION_ERROR:
            // This is for errors such as parity, framing, overrun or break (all 0 on the line)
            // None of these are fatal so we can ignore them
//            APP_ERROR_HANDLER(p_event->data.error_communication);
        break;
 
        case APP_UART_FIFO_ERROR:
                // Information only, we'll cope!
//            comm_uart_deinit();
//            start_reopen_timer();
//            APP_ERROR_HANDLER(p_event->data.error_code);
        break;
        
        case APP_UART_TX_EMPTY:
            // if tx had been full, can tell the user they can restart...
            if (_ctx.tx_ready_fn!=NULL) {
                (*_ctx.tx_ready_fn)(&comm_uart_tx);
            }
            break;
 
        default:
        break;
    }
}

/* to get libc to be ok */
static size_t
stdin_read(FILE *fp, char *bp, size_t n)
{
    return 0;
}

static size_t
stdout_write(FILE *fp, const char *bp, size_t n)
{
    comm_uart_tx((uint8_t*)bp, n,NULL);
    return n;
}

static struct File_methods _stdin_methods = {
    .write = stdout_write,
    .read = stdin_read
};

static struct File _stdin = {
    .vmt = &_stdin_methods
};

struct File *const stdin = &_stdin;
struct File *const stdout = &_stdin;
struct File *const stderr = &_stdin;


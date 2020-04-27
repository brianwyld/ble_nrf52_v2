/**
 * BSP functions for the Minex nRF51xxx module mounted on the Wyres dcard
 * Some of these are specific to the board, others for the nrf51 SOC
 */
#include "app_uart.h"
#include "bsp_minew_nrf51.h"

// UARTs config
static app_uart_comm_params_t _uart_comm_params[UART_CNT] =
    {{
        UART0_RX_PIN_NUMBER, // RX_PIN_NUMBER A
        UART0_TX_PIN_NUMBER, // TX_PIN_NUMBER A
        UART0_RTS_PIN_NUMBER,
        UART0_CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        115200,
    }
    };

// On this module we define 2 UARTs:
// - communication with external cards (AT command set processing) 
// - debug logging
bool hal_bsp_uart_init(int uartNb, int baudrate, app_uart_event_handler_t uart_event_handler) {
{
    uint32_t                     err_code;
    if (uartNb<0 || uartNb>=UART_CNT) {
        return NULL;
    }
    _uart_comm_params[uartNb].baud_rate = baudrate;
    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handler,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
    return &_uart_comm_params[uartNb];
}

void hal_bsp_uart_deinit(int uartNb) {
    if (uartNb<0 || uartNb >= UART_CNT) {
        return;
    }
    // only 1 uart in their api..
    app_uart_close();
}

// Tx line to uart. returns number of bytes not sent due to flow control
int hal_bsp_uart_tx(int uartNb, uint8_t d, int len) {
    if (uartNb<0 || uartNb >= UART_CNT) {
        return -1;      // fatal error
    }
    for(int i=0;i<len;i++) {
        // nrf uart api only allows one uart, sorry
        if (app_uart_put(d[i])!=NRF_SUCCESS) {
            return (len-i);       // Number of bytes not txd
        }
    }
    return 0;
}

// UART enable IO
bool hal_bsp_comm_uart_active_request() {
    // Should the external comm uart be init or deinit?
    return true;
}

// LED config
void hal_bsp_leds_init(void) {

}
void hal_bsp_leds_deinit(void) {
    
}

// buttons
void hal_bsp_buttons_init(void) {

}
void hal_bsp_buttons_deinit(void) {
    
}

// NVM for config management
// The NVM used for config on the nrf51 is pages of flash
// The NVM access functions are per byte or word, and the flash can only erase per 256 byte page... 
// This is mapped transparently for the caller by using a 'scratch flash' page during writing (to avoid using RAM)
static uint8_t _dirty[FLASH_PAGE_SZ];
static uint8_t* _dirtyAddr=NULL;
static bool writeFlashPage() {
    // Check if we have dirty scratch
    if (_dirtyAddr!=NULL) {
        // Which page is it?
        int page = (_dirtyAddr - FLASH_START) / FLASH_PAGE_SZ;
        // Start by erasing the page
        // TODO

        // And then allwoed to write to it
        for(int i=0; i<FLASH_PAGE_SZ;i++) {
            *(page*FLASH_PAGE_SZ + i) = _dirty[i];
        }
        _dirtyAddr = NULL;       // for next time
    }
}
uint16_t hal_bsp_nvmSize() {
    return FLASH_CONFIG_SZ;
}
// Lock flash, we're done writing
bool hal_bsp_nvmLock() {
        // write dirty scratch to it
        writeDirtyPage();
    return true;
}
// Unlock flash, we're going to be writing stuff
bool hal_bsp_nvmUnlock() {
    _dirtyAddr = NULL;       // nothing is dirty yet
    return true;
}

uint8_t hal_bsp_nvmRead8(uint16_t off) {
    return *((volatile uint8_t*)(FLASH_CONFIG_BASE_ADDR+off));
}
uint16_t hal_bsp_nvmRead16(uint16_t off) {
    return *((volatile uint16_t*)(FLASH_CONFIG_BASE_ADDR+off));
}
bool hal_bsp_nvmRead(uint16_t off, uint8_t len, uint8_t* buf) {
    for(int i=0;i<len;i++) {
        *(buf+i) = hal_bsp_nvmRead8(off+i);
    }
    return true;
}
bool hal_bsp_nvmWrite8(uint16_t off, uint8_t v) {
//    HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_BYTE, ((uint32_t)PROM_BASE)+off);
//    return (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_FASTBYTE, ((uint32_t)PROM_BASE)+off, v)==HAL_OK);
    // Sanity check
    if (off > hal_bsp_nvmSize()) {
        return false;       // no writing on the code
    }
    uint8_t *pageAddr = FLASH_CONFIG_BASE_ADDR+(off/FLASH_PAGE_SZ);
    // write to scratch page iff (!dirty || (dirty && off is in the-dirty-page)), set bit saying this byte is dirty
    if (_dirtyAddr==NULL) {
        _dirtyAddr = pageAddr;
        // initialise dirty page with current flash contents
        memcpy(_dirty, _dirtyAddr, FLASH_PAGE_SZ);
        // write the value to dirty scratch
        _dirty[off%FLASH_PAGE_SZ] = v;
    } else if (_dirtyAddr == pageAddr) {
        // same page as dirty one already, just write the byte
        _dirty[off%FLASH_PAGE_SZ] = v;
    } else {
        // Poo, its a different page
        // write dirty scratch to previous page
        writeDirtyPage();
        // reinit dirty with new page
        _dirtyAddr = pageAddr;
        // initialise dirty page with current flash contents
        memcpy(_dirty, _dirtyAddr, FLASH_PAGE_SZ);
        // write the value to dirty scratch
        _dirty[off%FLASH_PAGE_SZ] = v;
    }
    return true;
}
bool hal_bsp_nvmWrite16(uint16_t off, uint16_t v) {
    bool ret = true;
    // Just write as 2 bytes, no perf loss
    ret &= hal_bsp_nvmWrite8(off, v & 0xff);
    ret &= hal_bsp_nvmWrite8(off+1, (v>>8) & 0xff);
    return ret;
}
bool hal_bsp_nvmWrite(uint16_t off, uint8_t len, uint8_t* buf) {
    bool ret = true;
    for(int i=0;i<len;i++) {
        ret &= hal_bsp_nvmWrite8(off+i, *(buf+i));
    }
    return ret;
}


// ADC

void hal_bsp_adc_init(void)
{
    //NRF_ADC->POWER = ADC_POWER_POWER_Enabled << ADC_POWER_POWER_Pos;
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled << ADC_ENABLE_ENABLE_Pos;
    
    NRF_ADC->CONFIG = (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos) |
                      (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
                      (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) |
                      (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) |
                      (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);
}
 
void hal_bsp_adc_disable(void)
{
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled << ADC_ENABLE_ENABLE_Pos;
    //NRF_ADC->POWER = ADC_POWER_POWER_Disabled << ADC_POWER_POWER_Pos;
    
    NRF_ADC->CONFIG = (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) |
                      (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);
}
 
uint16_t hal_bsp_adc_read_u16(void)
{
    NRF_ADC->CONFIG     &= ~ADC_CONFIG_PSEL_Msk;
    NRF_ADC->CONFIG     |= ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos;
    NRF_ADC->TASKS_START = 1;
    while (((NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) >> ADC_BUSY_BUSY_Pos) == ADC_BUSY_BUSY_Busy) {};
    NRF_ADC->TASKS_STOP = 1;
    return (uint16_t)NRF_ADC->RESULT; // 10 bit
}

#if 0 
/* Interrupt handler for ADC data ready event. It will be executed when ADC sampling is complete */
void ADC_IRQHandler(void)
{
    /* Clear dataready event */
    NRF_ADC->EVENTS_END = 0;
}
#endif

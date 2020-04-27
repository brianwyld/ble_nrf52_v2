#ifndef H_MAIN_H
#define H_MAIN_H


/*********************************INCLUDE**************************************/

#include <inttypes.h>
#include <stdint.h>

/*******************************STRUCT & ENUM**********************************/
#define UUID16_SIZE             2                               /**< Size of 16 bit UUID */
#define UUID32_SIZE             4                               /**< Size of 32 bit UUID */
#define UUID128_SIZE            16                              /**< Size of 128 bit UUID */

#define PASSWORD_LEN    (4)

// Callback when output sink can take tx again (if flow controlled)
typedef int (*UART_TX_READY_FN_T)(UART_TX_FN_T tx_fn);
// Output line with N characters (may be null terminated or not)
typedef int (*UART_TX_FN_T)(uint8_t* output, int len, UART_TX_READY_FN_T txready);

/******************************GLOBAL FUNCTIONS********************************/

void system_init(void);

bool ibb_isBeaconning();
void ibb_start();
void ibb_stop();

/******************************GLOBAL VARIABLES********************************/
void cfg_setADV_IND(uint32_t value);
uint16_t cfg_getADV_IND( void );
void cfg_setTXPOWER_Level(int8_t level);
int8_t cfg_getTXPOWER_Level( void );
void cfg_setCompanyID(uint16_t value);
uint16_t cfg_getCompanyID( void );
uint16_t cfg_getMajor_Value( void );
uint16_t cfg_getMinor_Value( void );
void cfg_setMajor_Value( uint16_t );
void cfg_setMinor_Value( uint16_t );
bool cfg_checkPassword( char* given );
bool cfg_setPassword(char* oldp, char* newp);

int cfg_getFWMajor();
int cfg_getFWMinor();
//EOF
#endif  /* H_MAIN_H */
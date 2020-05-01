/**
 * AT command processing
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "ble_nus.h"
#include "app_util_platform.h"
#include "ble_hci.h"
#include "app_error.h"
#include "app_uart.h"
#include "ibs_scan.h"
#include "nrf_delay.h"
#include "softdevice_handler.h"

#include "wutils.h"
#include "device_config.h"

#include "main.h"
#include "at_process.h"
#include "comm_uart.h"

#define MAX_TXSZ (200)
#define MAX_RXSZ (200)

// per at command we have a definiton:
typedef enum { ATCMD_OK, ATCMD_GENERR, ATCMD_BADARG } ATRESULT;
typedef ATRESULT (*ATCMD_CBFN_t)(uint8_t nargs, char* argv[], void* odev);
typedef struct {
    const char* cmd;
    const char* desc;
    ATCMD_CBFN_t fn;
} ATCMD_DEF_t;

static const char* TYPE_SCANNER="1";
//static const char* TYPE_UART="2";     // Not required
static const char* TYPE_IBEACON="3";

// Write output to console on given device
static bool wconsole_println(void* udev, const char* l, ...);
static void at_process_line(char* line, UART_TX_FN_T utx_fn);

// predec at command handlers
static ATRESULT atcmd_hello(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_who(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_reset(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_listcmds(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_info(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_getcfg(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_setcfg(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_connect(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_disconnect(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_password(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_start_scan(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_stop_scan(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_start_ib(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_stop_ib(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_push(uint8_t nargs, char* argv[], void* odev);
static ATRESULT atcmd_type(uint8_t nargs, char* argv[], void* odev);

static ATCMD_DEF_t ATCMDS[] = {
    { .cmd="AT", .desc="Wakeup", .fn=atcmd_hello},
    { .cmd="AT+WHO", .desc="Card mode", .fn=atcmd_who},
    { .cmd="AT+HELP", .desc="List commands", .fn=atcmd_listcmds},
    { .cmd="ATZ", .desc="Reset card", .fn=atcmd_reset},
    { .cmd="AT+INFO", .desc="Show info", .fn=atcmd_info},
    { .cmd="AT+GETCFG", .desc="Show config", .fn=atcmd_getcfg},     
    { .cmd="AT+SETCFG", .desc="Set config", .fn=atcmd_setcfg},
    { .cmd="AT+VERSION", .desc="FW version", .fn=atcmd_info},         
    { .cmd="AT+CONN", .desc="Connect", .fn=atcmd_connect},  
    { .cmd="AT+DISC", .desc="Disconnect", .fn=atcmd_disconnect},  
    { .cmd="AT+PASS", .desc="Check password", .fn=atcmd_password},
    { .cmd="AT+START", .desc="Start scan", .fn=atcmd_start_scan},
    { .cmd="AT+STOP", .desc="Stop scan", .fn=atcmd_stop_scan},
    { .cmd="AT+IB_START", .desc="Start ibeaconning", .fn=atcmd_start_ib},
    { .cmd="AT+IB_STOP", .desc="Stop ibeaconning", .fn=atcmd_stop_ib},
    { .cmd="AT+PUSH", .desc="Push scan data", .fn=atcmd_push},
    { .cmd="AT+TYPE", .desc="Set card mode", .fn=atcmd_type},
};

// Our local data context
static struct {
    uint8_t txbuf[MAX_TXSZ+1];
    UART_TX_FN_T passThru_txfn1;        // If in passthru, this is output to one side
    UART_TX_FN_T passThru_txfn2;        // and this the other
    uint8_t ncmds;
    ATCMD_DEF_t* cmds;
} _ctx = {
    .cmds=ATCMDS,
    .ncmds= sizeof(ATCMDS)/sizeof(ATCMDS[0]),
};

// External api

// Deal with received data : line string either of max length or \r or \n terminated. utx_fn is the fn to send data back to this originator
// If in 'pass-thru' mode the data is just copied to the other guy (see AT+CONN) unless we find a "AT+DISC" in either direction
void at_process_input(char* data, UART_TX_FN_T source_txfn) {
    if (_ctx.passThru_txfn1!=NULL && _ctx.passThru_txfn2!=NULL) {
        // see which is the other guy..
        UART_TX_FN_T dest_txfn = _ctx.passThru_txfn1;  
        if (_ctx.passThru_txfn1==source_txfn) {
            // ah nope its the other
            dest_txfn = _ctx.passThru_txfn2;
        }
        // check for disconnect, else pass the data on
        if (strcmp("AT+DISC", data)==0) {
            (*dest_txfn)(NULL, -1, NULL);        // Tell remote dest we're done
            (*source_txfn)(NULL, -1, NULL);       // and confirm to source
            _ctx.passThru_txfn1 = NULL;      // ie disconnect
            _ctx.passThru_txfn2 = NULL;      // ie disconnect
        } else {
            // Give it all to dest
            if ((*dest_txfn)((uint8_t*)data, strlen(data), NULL)<0) {
                // broken
                (*source_txfn)(NULL, -1, NULL);        // tell guy who sent me data
                _ctx.passThru_txfn1 = NULL;      // ie disconnect
                _ctx.passThru_txfn2 = NULL;      // ie disconnect
            }
        }
    } else {
        // process it locally as AT command
        at_process_line(data, source_txfn);
    }
}

//Process an input line terminated by \0, and write any output back to the given tx fn
static void at_process_line(char* line, UART_TX_FN_T utx_fn) {
    // parse line into : command, args
    char* els[5];
    char* s = line;
    int elsi = 0;
    // first segment
    els[elsi++] = s;
    while (*s!=0 && elsi<5) {
        // Separators on line are space, = and , (so can accept "AT+X P1 P1", "AT+X=P1 P2" and "AT+X=P1,P2, P3" etc )
        if (*s==' ' || *s=='=' || *s==',') {
            // make end of string at white space
            *s='\0';
            s++;
            // consume blank space or seperators
            while(*s==' ' || *s=='=' || *s==',') {
                s++;
            }
            // If more string, then its the next element..
            if (*s!='\0') {
                els[elsi++] = s;
            } // else leave it on the end of string as we're done
        } else {
            s++;
        }
    }
    // find it in the list
    for(int i=0;i<_ctx.ncmds;i++) {
        if (strcmp(els[0], _ctx.cmds[i].cmd)==0) {
            // gotcha
            log_debug("got cmd %s with %d args", els[0], elsi-1);
            // call the specific command processor function as registered
            if ((*_ctx.cmds[i].fn)(elsi, els, utx_fn)!=ATCMD_OK) {
                // error message : should be determined by return
                wconsole_println(utx_fn, "ERROR\r\n Bad args for %s : %s", _ctx.cmds[i].cmd, _ctx.cmds[i].desc);
            } else {
                wconsole_println(utx_fn, "OK");
            }
            return;
        }
    }
    // not found
    wconsole_println(utx_fn, "ERROR\r\n Unknown command [%s].", els[0]);
    log_debug("no cmd %s with %d args", els[0], elsi-1);
}

// internals

// AT command processing
static ATRESULT atcmd_listcmds(uint8_t nargs, char* argv[], void* odev) {
    for(int i=0;i<_ctx.ncmds;i++) {
        wconsole_println(odev, "%s: %s", _ctx.cmds[i].cmd, _ctx.cmds[i].desc);
    }
    return ATCMD_OK;
}

static ATRESULT atcmd_hello(uint8_t nargs, char* argv[], void* odev) {
    wconsole_println(odev, "Hello.");
    return ATCMD_OK;
}

static ATRESULT atcmd_who(uint8_t nargs, char* argv[], void* odev) {
    if (ibb_isBeaconning()) {
        wconsole_println(odev, TYPE_IBEACON);
    } else {
        wconsole_println(odev, TYPE_SCANNER);
    }
    return ATCMD_OK;
}

static ATRESULT atcmd_reset(uint8_t nargs, char* argv[], void* odev) {
    app_reset_request();
    return ATCMD_OK;
}

static ATRESULT atcmd_info(uint8_t nargs, char* argv[], void* odev) {
    wconsole_println(odev, "Wyres BLE firmware v%d.%d", cfg_getFWMajor(), cfg_getFWMinor());
    wconsole_println(odev, "Scanning: %s, Beaconning: %s", (ibs_is_scan_active()?"YES":"NO"), (ibb_isBeaconning()?"YES":"NO"));
    return ATCMD_OK;
}

static void printKey(void* odev, uint16_t k, uint8_t* d, int l) {
    // Must explicitly check for illegal key (0000) as used for error check ie assert in configmgr
    if (k==DCFG_KEY_ILLEGAL) {
        wconsole_println(odev, "Key[%04x]=NOT FOUND", k);
        return;
    }
    // print element max data length 16
    switch(l) {
        case 0:
        case -1:  {
            wconsole_println(odev, "Key[%04x]=NOT FOUND", k);
            break;
        }
        case 1: {
            // print as decimal
            wconsole_println(odev, "Key[%04x]=%d", k, d[0]);
            break;
        }
        case 2: {
            // print as decimal
            uint16_t* vp = (uint16_t *)(&d[0]);     // avoid the overly keen anti-aliasing check
            wconsole_println(odev, "Key[%04x]=%d / 0x%04x", k, *vp, *vp);
            break;
        }
        case 4: {
            // print as decimal
            uint32_t* vp = (uint32_t *)(&d[0]);     // avoid the overly keen anti-aliasing check
            wconsole_println(odev, "Key[%04x]=%d / 0x%08x", k, *vp, *vp);
            break;
        }
        default: {
            // dump as hex up to 16 bytes
            char hs[34]; // 16*2 + 1 (terminator) + 1 (for luck)
            int sz = (l<16?l:16);       // in case its longer!
            for(int i=0;i<sz;i++) {
                sprintf(&hs[i*2], "%02x", d[i]);
            }
            hs[sz*2]='\0';
            wconsole_println(odev, "Key[%04x]=0x%s", k, hs);
            break;
        }
    }
}

static ATRESULT atcmd_getcfg(uint8_t nargs, char* argv[], void* odev) {
    // Check args - if 1 present then show just that config element else show all
    if (nargs>2) {
        return ATCMD_BADARG;
    }
    if (nargs==1) {
        // get al config elements and print them
        // Currently not possible as uart output buffer can't hold them all
        cfg_iterateKeys(odev, &printKey);
    } else if (nargs==2) {
        int k=0;
        int kl = strlen(argv[1]);
        if (kl==4) {
            if (sscanf(argv[1], "%4x", &k)<1) {
                wconsole_println(odev, "Bad key [%s] must be 4 hex digits", argv[1]);
                return ATCMD_BADARG;
            } else {
                uint8_t d[16];
                int l = cfg_getByKey(k, &d[0], 16);
                printKey(odev, k, &d[0], l);
            }
        } else {
            wconsole_println(odev, "Error : Must give key of 4 digits");
            return ATCMD_BADARG;
        }
    }
    return ATCMD_OK;
}
static ATRESULT atcmd_setcfg(uint8_t nargs, char* argv[], void* odev) {
    // Must have done a password login to set params
    if (!cfg_isPasswordOk()) {
        return ATCMD_GENERR;
    }

    // args : cmd, config key (always 4 digits hex), config value as hex string (0x prefix) or decimal
    if (nargs<3) {
        return ATCMD_BADARG;
    }
    // parse key
    if (strlen(argv[1])!=4) {
        wconsole_println(odev, "Key[%s] must be 4 digits", argv[1]);
        return ATCMD_BADARG;
    }
    int k=0;
    if (sscanf(argv[1], "%4x", &k)<1) {
        wconsole_println(odev, "Key[%s] must be 4 digits", argv[1]);
    } else {
        // parse value
        // Get the length it should be by a dummy get
        uint32_t d=0;
        int l = cfg_getByKey(k, (uint8_t*)&d, 4);
        switch(l) {
            case 0: {
                wconsole_println(odev, "Key[%s] does not exist", argv[1]);
                // TODO ? create in this case? maybe if a -c arg at the end? maybe not useful?
                break;
            }
            case 1: 
            case 2:
            case 3:
            case 4: {
                int v = 0;
                char *vp = argv[2];
                if (*vp=='0' && *(vp+1)=='x') {
                    if (strlen((vp+2))!=(l*2)) {
                        wconsole_println(odev, "Key[%04x]=%s value is incorrect length. (expected %d bytes)", k, argv[2], l);
                        return ATCMD_BADARG;
                    }
                    if (sscanf((vp+2), "%x", &v)<1) {
                        wconsole_println(odev, "Key[%04x] bad hex value:%s",k,vp);
                        return ATCMD_BADARG;
                    }
                } else {
                    if (sscanf(vp, "%d", &v)<1) {
                        wconsole_println(odev, "Key[%04x] bad dec value:%s",k,vp);
                        return ATCMD_BADARG;
                    }
                }
                cfg_setByKey(k, (uint8_t*)&v, l);
                printKey(odev, k, (uint8_t*)&v, l);        // Show the value now in the config 
                break;
            }

            default: {
                // parse as hex string
                char* vp = argv[2];
                // Skip 0x if user put it in
                if (*vp=='0' && *(vp+1)=='x') {
                    vp+=2;
                }
                //Check got enough digits
                if (strlen(vp)!=(l*2)) {
                    wconsole_println(odev, "Key[%04x]=%s value is incorrect length. (expected %d bytes)", k, argv[2], l);
                    return ATCMD_BADARG;
                }
                if (l>16) {
                    wconsole_println(odev, "Key[%04x] has length %d : cannot set (max 16)", k, l);
                    return ATCMD_BADARG;
                }
                // gonna allow up to 16 bytes
                uint8_t val[16];
                for(int i=0;i<l;i++) {
                    // sscanf into int (4 bytes) then copy just LSB as value for each byte
                    unsigned int b=0;
                    if (sscanf(vp, "%02x", &b)<1) {
                        wconsole_println(odev, "Key[%04x] bad hex : %s", k, vp);
                        return ATCMD_BADARG;
                    }
                    val[i] = b;
                    vp+=2;
                }
                cfg_setByKey(k, &val[0], l);
                printKey(odev, k, &val[0], l);
                break;
            }
        }
    }
    return ATCMD_OK;
}

// Request to cross-connect to another comm port
// This may be from the BLE uart service, to the UART port (classic) if no params are given
// or from uart to a remote BLE client (if relevant addresses given) : TODO
static ATRESULT atcmd_connect(uint8_t nargs, char* argv[], void* odev) {
    if (nargs==1) {
        // the sender is one side (assumed to be a remote BLE) and the comm UART is forced as the other side
        _ctx.passThru_txfn1 = (UART_TX_FN_T)odev;
        _ctx.passThru_txfn2 = &comm_uart_tx;
        // Setting these 2 attributes will mean that the pass-thru handling takes place in the at_process_line() method
        wconsole_println(odev, "OK");
    } else {
        wconsole_println(odev, "NOK : not yet implemented");
    }
    return ATCMD_OK;
}
// And tear down the cross-connect
static ATRESULT atcmd_disconnect(uint8_t nargs, char* argv[], void* odev) {
    // Are we connected currently?
    if (_ctx.passThru_txfn1!=NULL && _ctx.passThru_txfn2!=NULL) {
        // yes, tell disconnect to other guy
        if (_ctx.passThru_txfn1==(UART_TX_FN_T)odev) {
            (*_ctx.passThru_txfn2)(NULL, -1, NULL);       // ie disconnected
        } else {
            (*_ctx.passThru_txfn1)(NULL, -1, NULL);       // ie disconnected
        }
        _ctx.passThru_txfn1 = NULL;
        _ctx.passThru_txfn2 = NULL;
        wconsole_println(odev, "OK");
    } else {
        wconsole_println(odev, "NOK : Not connected");
    }
//    sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    return ATCMD_OK;
}

static ATRESULT atcmd_password(uint8_t nargs, char* argv[], void* odev) {
    if (nargs==2) {
        // Check password
        if (cfg_checkPassword(argv[1])) {
            return ATCMD_OK;
        }
    } else if (nargs==3) {
        // set password (with old one first)
        if (cfg_setPassword(argv[1], argv[2])) {
            return ATCMD_OK;
        }
    }
    // Else not
    cfg_resetPasswordOk();
    return ATCMD_GENERR;
}

static ATRESULT atcmd_start_scan(uint8_t nargs, char* argv[], void* odev) {
    // check if a UUID is provided
    // expected pattern is AT+START,<UUID> UUID is 32 hex digits long
    // parser cuts into argv[0]=AT+START, argv[1]=<UUID>
    if (nargs>1) 
    {
        // Parse hex string to build bytes
        uint8_t uuid[16];
        if (Util_scanhex(argv[1], 16, uuid)!=16) 
//        if (sscanf(argv[1], "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
//                    &uuid[0],&uuid[1],&uuid[2],&uuid[3],&uuid[4],&uuid[5],&uuid[6],&uuid[7],
//                    &uuid[8],&uuid[9],&uuid[10],&uuid[11],&uuid[12],&uuid[13],&uuid[14],&uuid[15])!=16) 
        {
            // oops
            wconsole_println(odev, "ERROR : failed to parse UUID [%s]", argv[1]);
            return ATCMD_BADARG;
        }
        
        ibs_scan_set_uuid_filter(&uuid[0]);
    }
    else
    {
        ibs_scan_set_uuid_filter(NULL);
    }
    
    if (!ibs_scan_start((UART_TX_FN_T)odev)) 
    {
        return ATCMD_GENERR;
    }
    return ATCMD_OK;
}

static ATRESULT atcmd_stop_scan(uint8_t nargs, char* argv[], void* odev) {
    if (!ibs_scan_stop()) 
    {
        return ATCMD_GENERR;
    }
    return ATCMD_OK;
}

static ATRESULT atcmd_start_ib(uint8_t nargs, char* argv[], void* odev) {
    // set all the params from the args optionally
    if (nargs==7) {
        // AT_IB_START <uuid>,<major>,<minor>,<extrabyte>,<interval in ms>,<txpower>
        // All values in hex
        uint8_t uuid[16];
        unsigned int major;
        unsigned int minor;
        unsigned int extra;
        unsigned int interMS;
        int txpower;
        if (Util_scanhex(argv[1], 16, uuid)!=16) 
//        if (sscanf(argv[1], "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
//                    &uuid[0],&uuid[1],&uuid[2],&uuid[3],&uuid[4],&uuid[5],&uuid[6],&uuid[7],
//                    &uuid[8],&uuid[9],&uuid[10],&uuid[11],&uuid[12],&uuid[13],&uuid[14],&uuid[15])!=16) 
        {
            // oops
            wconsole_println(odev, "ERROR : failed to parse UUID [%s]", argv[1]);
            return ATCMD_BADARG;
        }
        if (sscanf(argv[2], "%04x", &major)!=1) 
        {
            wconsole_println(odev, "ERROR : failed to parse major[%s]", argv[2]);
            return ATCMD_BADARG;
        }
        if (sscanf(argv[3], "%04x", &minor)!=1) 
        {
            wconsole_println(odev, "ERROR : failed to parse minor[%s]", argv[3]);
            return ATCMD_BADARG;
        }
        if (sscanf(argv[4], "%02x", &extra)!=1) 
        {
            wconsole_println(odev, "ERROR : failed to parse extra[%s]", argv[4]);
            return ATCMD_BADARG;
        }
        if (sscanf(argv[5], "%04x", &interMS)!=1) 
        {
            wconsole_println(odev, "ERROR : failed to parse interval[%s]", argv[5]);
            return ATCMD_BADARG;
        }
        if (sscanf(argv[6], "%d", &txpower)!=1) 
        {
            wconsole_println(odev, "ERROR : failed to parse tx power[%s]", argv[6]);
            return ATCMD_BADARG;
        }
        cfg_setUUID(uuid);
        cfg_setMajor_Value(major);
        cfg_setMinor_Value(minor);
        cfg_setExtra_Value(extra);
        cfg_setADV_IND(interMS);
        cfg_setTXPOWER_Level(txpower);
    }
    ibb_start();
    return ATCMD_OK;
}

static ATRESULT atcmd_stop_ib(uint8_t nargs, char* argv[], void* odev) {
    ibb_stop();
    return ATCMD_OK;
}

static ATRESULT atcmd_push(uint8_t nargs, char* argv[], void* odev) {
    // always in push mode anyway
    return ATCMD_OK;
}
static ATRESULT atcmd_type(uint8_t nargs, char* argv[], void* odev) {
    // If type set to scanner, then stop ibeaconning and vice-versa
    if (nargs==2) {
        if (strcmp(argv[1], TYPE_SCANNER)==0) {
            ibb_stop();
        }
        if (strcmp(argv[1], TYPE_IBEACON)==0) {
            ibs_scan_stop();
        }
    }
    return ATCMD_OK;
}

// internal processing
static bool wconsole_println(void* dev, const char* l, ...) {
    UART_TX_FN_T utx_fn = (UART_TX_FN_T)dev;
    bool ret = true;
    va_list vl;
    va_start(vl, l);
    vsprintf((char*)&_ctx.txbuf[0], l, vl);
    int len = strlen((const char*)&_ctx.txbuf[0]);  //, MAX_TXSZ);        why no strnlen in gcc libc??
    if ((len)>=MAX_TXSZ) {
        // oops might just have broken stuff...
        _ctx.txbuf[MAX_TXSZ-1] = '\0';
        ret = false;        // caller knows there was an issue
    } else {
        _ctx.txbuf[len]='\n';
        _ctx.txbuf[len+1]='\r';
        _ctx.txbuf[len+2]='\0';
        len+=3;
    }
    if (utx_fn!=NULL) {
        int res = (*utx_fn)(&_ctx.txbuf[0], len, NULL);
        if (res<len) {
            _ctx.txbuf[0] = '*';
            (*utx_fn)(&_ctx.txbuf[0], 1, NULL);      // so user knows he missed something.
            ret = false;        // caller knows there was an issue
            // Not dealing with tx ready upcalls just now
           log_noout_fn("console write FAIL %d", res);      // just for debugger to watch
       }
    } else {
        ret = false;        // caller knows there was an issue
        log_noout_fn("console write no open uart");      // just for debugger to watch
    }
    va_end(vl);
    return ret;
}

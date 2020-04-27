#ifndef WAT_CMD_H__
#define WAT_CMD_H__

#include <stdint.h>
#include <stdbool.h>

#define IBS_AT_CMD_MAX_LEN 50

typedef struct
{
    char* name;
    void (*handler)(uint8_t *data);
} wat_cmd_t;

#define GLOBAL_LOCATION     0x338A0
#define PASSWORD_LOCATION   0x338b4//0x000028400
#define MINOR_LOCATION      0x338d8
#define MAJOR_LOCATION      0x338fc
#define POWER_LOCATION      0x33924
#define ADVINT_LOCATION     0x33938
#define TYPE_LOCATION       0x33CA0

void wat_process_at_version(uint8_t *data);
void wat_process_at_advint(uint8_t *data);
void wat_process_at_disconnect(uint8_t *data);
void wat_process_at_txpower(uint8_t *data);
void wat_process_at_minor(uint8_t *data);
void wat_process_at_major(uint8_t *data);
void wat_process_at_password(uint8_t *data);
void wat_process_at_disconnect(uint8_t *data);
void wat_process_at_get_config(uint8_t *data);
void wat_process_at_company_id(uint8_t *data);
void wat_process_at_uuid(uint8_t *data);
void wat_process_at_name(uint8_t *data);

bool wat_set_advind(uint8_t* interval);
bool wat_set_txpower(uint8_t *txpower);
bool wat_set_minor( uint8_t *minor);
bool wat_set_major( uint8_t *major);
bool wat_set_company_id(uint8_t *id);
bool wat_set_uuid(uint8_t *uuid);
bool wat_set_uuid2(uint8_t *uuid);
bool wat_set_uuid3(uint8_t *uuid);
bool wat_set_name(uint8_t *name);
uint8_t wat_activate_password(uint8_t *password);
uint8_t wat_write_password(uint8_t *new_password);

uint8_t wat_write_flash_password(uint8_t *new_password);
uint8_t wat_write_flash_minor(uint16_t value);
uint8_t wat_write_flash_major(uint16_t value);
uint8_t wat_write_flash_txpower(int8_t value);
uint8_t wat_write_flash_advint(uint16_t value);
uint8_t wat_write_flash_type(uint8_t device_type);

void wat_get_flash_major(uint16_t *value);
void wat_get_flash_minor(uint16_t *value);
void wat_get_flash_password(uint8_t *tab);
void wat_get_flash_txwpower(int8_t *value);
void wat_get_flash_advint(uint16_t *value);
void wat_get_flash_type(uint8_t *value);

void wat_get_flash_config(void);
uint8_t wat_write_flash_config(uint8_t *new_password, uint16_t major, uint16_t minor, int8_t power, uint16_t adv);

void wat_parse_at_cmd(uint8_t *data);
void wat_send_nus(char string[]);

#endif // WAT_CMD_H__

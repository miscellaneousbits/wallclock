#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    struct server* server;
    void* buffer;
    uint32_t len;
} user_data_t;

struct server
{
    int fd;
    struct bt_att* att;
    struct gatt_db* db;
    struct bt_gatt_server* gatt;

    char* device_name;
    size_t name_len;

    uint16_t gatt_svc_chngd_handle;
    char svc_chngd_enabled;

    uint16_t clock_read_data_handle;
    uint16_t clock_write_data_handle;
    uint8_t clock_read_data_notify;
};

void* server_thread(void* ptr);
char clock_read_data_cb(void* user_data);

#ifndef _MAIN_H_
#define _MAIN_H_

#define LOG_ERR_STR "<3>"
#define LOG_WARNING_STR "<4>"
#define LOG_INFO_STR "<6>"

#define MINS2SECS(_m_) ((_m_) * 60u)
#define HRS2SECS(_h_) MINS2SECS((_h_) * 60u)

#define NUM_POTS 3


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <systemd/sd-daemon.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib/uuid.h"

#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/timeout.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"

#include "led.h"

struct server {
    int fd;
    struct bt_att *att;
    struct gatt_db *db;
    struct bt_gatt_server *gatt;

    char *device_name;
    size_t name_len;

    uint16_t gatt_svc_chngd_handle;
    bool svc_chngd_enabled;

    uint16_t i2c_read_data_handle;
    uint16_t i2c_write_data_handle;
    uint8_t i2c_read_data_notify;
};

typedef struct {
    struct server *server;
    void *buffer;
    uint32_t len;
} user_data_t;

#include "common-types.h"
#include "clock-messages.h"
#include "clock-commands.h"
#include "server.h"

#endif


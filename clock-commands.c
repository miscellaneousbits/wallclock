#include "main.h"

volatile time_t gLastPollTime;
volatile uint16_t gPollInterval;
volatile int64_t gDelta;
volatile uint16_t gVbat;
volatile uint16_t gMatch;
volatile uint16_t gPollCount;
volatile uint16_t gTimeouts;
uint64_t gFaceTime;

uint64_t get_time(void)
{
    struct tm t;
    time_t tt = time(NULL);
    localtime_r(&tt, &t);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return
        ((uint64_t)ts.tv_sec << 8) + (((uint64_t)ts.tv_nsec << 8) / 1000000000U) +
        (t.tm_gmtoff << 8);
}

int get_clock_face_time(char *ts, uint64_t *t)
{
    uint32_t h, m, s;

    if (sscanf(ts, "%u:%u:%u", &h, &m, &s) != 3)
        return -1;
    if (h > 23)
        return -1;
    if (m > 59)
        return -1;
    if (s > 59)
        return -1;
    printf(LOG_INFO_STR "Starting clock at %02u:%02u:%02u\n", h, m, s);
    fflush(stdout);
    time_t currtime;
    struct tm timeinfoLocal;

    currtime = time(NULL);
    localtime_r(&currtime, &timeinfoLocal);
    timeinfoLocal.tm_hour = h;
    timeinfoLocal.tm_min = m;
    timeinfoLocal.tm_sec = s;
    *t = ((uint64_t)mktime(&timeinfoLocal) << 8u) + (timeinfoLocal.tm_gmtoff << 8);;
    return 0;
}

void clock_command(struct server *server, const uint8_t *cmd, uint8_t len)
{
    msg_t m;
    user_data_t user_data;
    static time_t lastActivity;

    if (len > sizeof(msg_t))
        len = sizeof(msg_t);
    memcpy(&m, cmd, len);
    switch (m.u8.cmd) {
    case CLK_REQ_FACE_TIME:
        if (gFaceTime) {
            m.u64.cmd = SRVR_ACCEPT_FACE_TIME;
            m.u64.data = gFaceTime;
            //i2c_write(BLE_I2C_SLAVE_ADDRESS, &m, sizeof(msg_u64_t));
            user_data.server = server;
            user_data.buffer = &m;
            user_data.len = sizeof(msg_u64_t);
            i2c_read_data_cb(&user_data);
            printf(LOG_INFO_STR "Face time set %016llx\n", gFaceTime);
            fflush(stdout);
            lastActivity = time(NULL);
            break;
        }
        fprintf(stderr, LOG_ERR_STR "Face time requested, but not set by cmd line\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
        break;

    case CLK_REQ_TIME:
        m.u64.cmd = SRVR_ACCEPT_TIME;
        m.u64.data = get_time();
        //i2c_write(BLE_I2C_SLAVE_ADDRESS, &m, sizeof(msg_u64_t));
        user_data.server = server;
        user_data.buffer = &m;
        user_data.len = sizeof(msg_u64_t);
        i2c_read_data_cb(&user_data);
        break;

    case CLK_ACCEPT_STATUS:
        gPollInterval = 1 << (m.sts.interval + 5);
        gDelta = m.sts.offset;
        gVbat = m.sts.battery;
        gMatch = m.sts.match;
        gLastPollTime = time(NULL);
        gPollCount++;
        gTimeouts = m.sts.timeouts;
        m.u8.cmd = SRVR_ACCEPT_DONE;
        user_data.server = server;
        user_data.buffer = &m;
        user_data.len = sizeof(msg_u8_t);
        i2c_read_data_cb(&user_data);
        float b = (gVbat * 0.0011) +  0.004;
        b = (b - 3.3) / (4.5 - 3.3);
        if (b < 0.0)
            b = 0.0;
        printf(LOG_INFO_STR
               "CI=%u,SI=%u,TO=%u,PC=%u,D=%0.2f,V=%0.2f,M=%u\n",
               gPollInterval,
               (uint32_t)(gLastPollTime - lastActivity),
               gTimeouts,
               gPollCount,
               gDelta / 256.0,
               (gVbat * 0.0011) + 0.0013,
               gMatch
              );
        fflush(stdout);
        lastActivity = gLastPollTime;
        if (gFaceTime) {
            printf(LOG_INFO_STR "Face time set acknowledged\n");
            fflush(stdout);
            gFaceTime = 0;
        }
        break;
    }
}

#include "main.h"

#define OLED_I2C_ADDR 0x3c

typedef unsigned char uint8_t;

#include "i2c.h"
#include "main.h"
#include "clock-monitor.h"
#include "clock-commands.h"
#include "gfx.h"
#include "ssd1306.h"


static uint16 lastPollInterval;
static int64 lastDelta = 9223372036854775807;
static uint16 lastVbat;
static uint16 lastMatch;
static uint16 lastTimeouts = 1000;

static volatile bool bail = false;

static void term(int signum)
{
    (void)signum;
    bail = true;
}

void *mon_thread(void *param)
{
    printf(LOG_INFO_STR "Time monitor: started\n");
    fflush(stdout);

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, (int)param);
    int rc = pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);
    if (rc != 0) {
        fprintf(stderr, LOG_ERR_STR "Error setting signal mask\n");
        fflush(stderr);
        return NULL;
    }

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction((int)param, &action, NULL);

    while (!bail) {

        char buffer[17];

        time_t timer;
        struct tm *tm_info;

        time(&timer);
        tm_info = localtime(&timer);
        strftime(buffer, sizeof(buffer), "%I:%M:%S", tm_info);
        gfx_setCursor(0, 0);
        gfx_setTextSize(2);
        gfx_writeStr(buffer);
        ssd1306_displayLine(0);
        ssd1306_displayLine(1);

        if (gDelta != lastDelta) {
            gfx_setCursor(0, 16);
            gfx_writeStr("        ");
            gfx_setCursor(0, 16);
            snprintf(buffer, sizeof(buffer), "D %0.2f", gDelta / 256.0);
            gfx_writeStr(buffer);
            lastDelta = gDelta;
            ssd1306_displayLine(2);
            ssd1306_displayLine(3);
        }
        if (gVbat != lastVbat) {
            gfx_setCursor(0, 32);
            gfx_writeStr("        ");
            gfx_setCursor(0, 32);
            snprintf(buffer, sizeof(buffer), "V %0.2f", gVbat * 0.0011 + 0.0013);
            gfx_writeStr(buffer);
            lastVbat = gVbat;
            ssd1306_displayLine(4);
            ssd1306_displayLine(5);
        }
        gfx_setTextSize(1);
        if ((gPollInterval != lastPollInterval) ||
            (gTimeouts != lastTimeouts) ||
            (gMatch != lastMatch)) {
            gfx_setCursor(0, 56);
            gfx_writeStr("                ");
            gfx_setCursor(0, 56);
            snprintf(buffer, sizeof(buffer), "I=%u T=%u %u", gPollInterval, gTimeouts, gMatch);
            gfx_writeStr(buffer);
            lastPollInterval = gPollInterval;
            ssd1306_displayLine(7);
        }

        struct timespec s;
        clock_gettime(CLOCK_MONOTONIC, &s);
        uint32 us = s.tv_nsec / 1000;
        usleep(1000000 - us);
    }

    printf(LOG_INFO_STR "Time monitor: stopped\n");
    fflush(stdout);
    pthread_exit(NULL);
}


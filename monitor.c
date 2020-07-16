
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "led.h"
#include "log.h"

static volatile char bail = 0;
static time_t last_t;

static void term(int signum)
{
    (void)signum;
    bail = 1;
}

static volatile unsigned timeleft = 0;

void monitor_set_poll_interval(int secs)
{
    timeleft = secs + 15;
}

static void clock_alarm(void)
{
    led_red(1);
    led_green(0);
    led_blue(0);
    led_blink(1);
    printf(LOG_ERR_STR "Clock monitor: Lost contact\n");
}

void* monitor_thread(void* ptr)
{
    printf(LOG_INFO_STR "Clock monitor: started\n");
    fflush(stdout);

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, (int)ptr);
    int rc = pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);
    if (rc != 0)
    {
        fprintf(stderr, LOG_ERR_STR "Error setting signal mask\n");
        return NULL;
    }

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction((int)ptr, &action, NULL);
    last_t = time(NULL);
    while (!bail)
    {
        sleep(1);
        time_t n = time(NULL);
        unsigned d = n - last_t;
        last_t = n;
        if (timeleft)
        {
            if (timeleft < d)
                timeleft = d;
            timeleft -= d;
            if (timeleft == 0)
                clock_alarm();
        }
    }
    printf(LOG_INFO_STR "Clock monitor: stopped\n");
    return NULL;
}

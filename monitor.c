
#include "main.h"

static volatile bool bail = false;

static void term(int signum)
{
    (void)signum;
    bail = true;
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

    while (!bail)
    {
        sleep(1);
    }
    printf(LOG_INFO_STR "Clock monitor: stopped\n");
    return NULL;
}

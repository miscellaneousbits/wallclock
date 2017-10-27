#include "main.h"

static pthread_t mon_thread_handle;
static pthread_t server_thread_handle;

static void term(int signum)
{
    (void)signum;
    pthread_kill(mon_thread_handle, SIGUSR1);
    pthread_kill(server_thread_handle, SIGUSR2);
}

int main(int argc, char *argv[])
{
    printf(LOG_INFO_STR "Starting\n");
    fflush(stdout);
    if ((argc >= 3) && (strcmp(argv[1], "-s") == 0)) {
        if (get_clock_face_time(argv[2], &gFaceTime) < 0) {
            fprintf(stderr, "Bad face time format\n");
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGINT);
    sigaddset(&signal_mask, SIGTERM);
    sigaddset(&signal_mask, SIGUSR1);
    sigaddset(&signal_mask, SIGUSR2);
    int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0) {
        fprintf(stderr, LOG_ERR_STR "Error setting signal mask\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    i2c_init();
    ssd1306_checkOled();

    if (pthread_create(&mon_thread_handle, NULL, mon_thread, (void *)SIGUSR1)) {
        fprintf(stderr, LOG_ERR_STR "Error creating BLE server thread\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    if (pthread_create(&server_thread_handle, NULL, server_thread, (void *)SIGUSR2)) {
        fprintf(stderr, LOG_ERR_STR "Error creating BLE server thread\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    rc = pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);
    if (rc != 0) {
        fprintf(stderr, LOG_ERR_STR "Error setting signal mask\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    sd_notify(0, "READY=1");

    if (pthread_join(mon_thread_handle, NULL)) {
        fprintf(stderr, LOG_ERR_STR "Error joining monitor thread\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    if (pthread_join(server_thread_handle, NULL)) {
        fprintf(stderr, LOG_ERR_STR "Error joining monitor thread\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    printf(LOG_INFO_STR "Stopped\n");
    fflush(stdout);

    return EXIT_SUCCESS;
}

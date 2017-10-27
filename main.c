#include "main.h"

char ba[18];

int main(int argc, char *argv[])
{
    printf(LOG_INFO_STR "Starting wallclock server\n");
    if ((argc >= 3) && (strcmp(argv[1], "-s") == 0)) {
        if (get_clock_face_time(argv[2], &gFaceTime) < 0) {
            fprintf(stderr, "Bad face time format\n");
            exit(EXIT_FAILURE);
        }
    }


    i2c_init();
    ssd1306_checkOled();

    pthread_t mon_thread_handle;
    if (pthread_create(&mon_thread_handle, NULL, mon_thread, NULL)) {
        fprintf(stderr, LOG_ERR_STR "Error creating BLE server thread\n");
        return EXIT_FAILURE;
    }

    pthread_t server_thread_handle;
    if (pthread_create(&server_thread_handle, NULL, server_thread, NULL)) {
        fprintf(stderr, LOG_ERR_STR "Error creating BLE server thread\n");
        return EXIT_FAILURE;
    }

    sd_notify(0, "READY=1");

    if (pthread_join(mon_thread_handle, NULL)) {
        fprintf(stderr, LOG_ERR_STR "Error joining monitor thread\n");
        return EXIT_FAILURE;
    }

    printf(LOG_INFO_STR "Wallclock server stopped\n");

    return EXIT_SUCCESS;
}

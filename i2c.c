#include "main.h"
#include <linux/i2c-dev.h>

#define DEBUG 0
#define ENABLE_I2C 1

#if ENABLE_I2C
static int file;
#endif

int i2c_init(void)
{
#if ENABLE_I2C

    if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
        fprintf(stderr, LOG_ERR_STR "Failed to open the bus.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return -1;
    }
#endif
    return 0;
}


static void i2c_slave_address(int addr)
{
#if ENABLE_I2C
#if DEBUG
    printf(LOG_INFO_STR "slave addr = %02x\n", addr);
#endif
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, LOG_ERR_STR "Failed to acquire bus access and/or talk to slave - 0x%02x.\n", addr);
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }
#endif
}

void i2c_close(void)
{
#if ENABLE_I2C
    close(file);
#endif
}


int i2c_read(int addr, void *buf, int nbytes)
{
#if ENABLE_I2C
    i2c_slave_address(addr);
    if (read(file, buf, nbytes) != nbytes) {
        fprintf(stderr, LOG_ERR_STR "Error reading %i bytes from slave 0x%02x\n", nbytes, addr);
        return -1;
    }
#if DEBUG
    printf(LOG_INFO_STR "read =");
    int i;
    for (i = 0; i < nbytes; i++)
        printf(LOG_INFO_STR " %02x", ((char *)buf)[i]);
    printf(LOG_INFO_STR "\n");
#endif
    return nbytes;
#else
    return 0;
#endif
}

void i2c_write(int addr, void *buf, int nbytes)
{
#if ENABLE_I2C
    i2c_slave_address(addr);
#if DEBUG
    printf(LOG_INFO_STR "write =");
    int i;
    for (i = 0; i < nbytes; i++)
        printf(stderr, LOG_INFO_STR  " %02x", ((char *)buf)[i]);
    printf(stderr, LOG_INFO_STR  "\n");
#endif
    if (write(file, buf, nbytes) != nbytes)
        printf(LOG_INFO_STR "Error writing %i bytes to slave 0x%02x\n", nbytes, addr);
#endif
}


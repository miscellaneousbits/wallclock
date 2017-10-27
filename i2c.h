#ifndef _i2c_h_
#define _i2c_h_

int i2c_init(void);
int i2c_read(int addr, void *buf, int nbytes);
void i2c_write(int addr, void *buf, int nbytes);
void i2c_close(void);

#endif

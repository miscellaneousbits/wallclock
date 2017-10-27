#include "main.h"

#define OLED_I2C_ADDR 0x3c

extern int run_daemon;

static uint8 i2caddr;

static uint16 wBufUsed;
static uint8 mirror;
static uint8 wBuf[1024];
static uint8 ymirror[256];

static void init_ymirror(void)
{
    int i;
    for (i = 0; i < 256; i++) {
        uint8 ii = i, k = 0;
        int j;
        for (j = 0; j < 8; j++) {
            k <<= 1;
            if (ii & 1)
                k |= 1;
            ii >>= 1;
        }
        ymirror[i] = k;
    }
}

static void Oled_begin(void)
{
    wBufUsed = 0;
}

void Oled_write(uint8 control)
{
    wBuf[wBufUsed++] = control;
}

void Oled_write_block(uint8 *data, uint16 len)
{
    memcpy(wBuf + wBufUsed, data, len);
    wBufUsed += len;
}

void Oled_end(void)
{
    i2c_write(i2caddr, wBuf, wBufUsed);
}

// the memory buffer for the LCD

static uint8 buffer[(SSD1306_LCDHEIGHT *SSD1306_LCDWIDTH) / 8];

#define ssd1306_swap(a, b) { short t = a; a = b; b = t; }

// the most basic function, set a single pixel
void ssd1306_drawPixel(short x, short y, uint16 color)
{
    if ((x < 0) || (x >= SSD1306_LCDWIDTH) || (y < 0) || (y >= SSD1306_LCDHEIGHT))
        return;

    // x is which column
    switch (color) {
    case WHITE:
        buffer[x + (y / 8)*SSD1306_LCDWIDTH] |= (1 << (y & 7));
        break;
    case BLACK:
        buffer[x + (y / 8)*SSD1306_LCDWIDTH] &= ~(1 << (y & 7));
        break;
    case INVERSE:
        buffer[x + (y / 8)*SSD1306_LCDWIDTH] ^= (1 << (y & 7));
        break;
    }

}

static void ssd1306_command(uint8 c)
{
    Oled_begin();
    Oled_write(0);
    Oled_write(c);
    Oled_end();
}

static const uint8 ssd1306_init_commands[] = {
    SSD1306_DISPLAYOFF,                    // 0xAE

    SSD1306_SETDISPLAYCLOCKDIV,            // 0xD5
    0x80,                                  // the suggested ratio 0x80

    SSD1306_SETMULTIPLEX,                  // 0xA8
    SSD1306_LCDHEIGHT - 1,

    SSD1306_SETDISPLAYOFFSET,              // 0xD3
    0x0,                                   // no offset

    SSD1306_SETSTARTLINE | 0x0,            // line #0

    SSD1306_CHARGEPUMP,                    // 0x8D
    0x14,

    SSD1306_MEMORYMODE,                    // 0x20
    0x02,                                  // 0x2 page mode

    SSD1306_SEGREMAP | 0x1,		    // 0xA1

    SSD1306_COMSCANDEC,		    	    // 0xC8

    SSD1306_SETCOMPINS,                    // 0xDA
    0x12,

    SSD1306_SETCONTRAST,                   // 0x81
    0xCF,

    SSD1306_SETPRECHARGE,                  // 0xd9
    0xF1,

    SSD1306_SETVCOMDETECT,                 // 0xDB
    0x40,

    SSD1306_DISPLAYALLON_RESUME,           // 0xA4

    SSD1306_NORMALDISPLAY,                 // 0xA6

    SSD1306_SETPAGESTART,		    // 0xB0
    SSD1306_SETHIGHCOLUMN,		    // 0x10
    SSD1306_SETLOWCOLUMN,	            // 0x00

    SSD1306_DISPLAYON  //--turn on oled     // 0xaf
};

static const uint8 ssd1306_init_commands_mirror[] = {
    SSD1306_DISPLAYOFF,                    // 0xAE

    SSD1306_SETDISPLAYCLOCKDIV,            // 0xD5
    0x80,                                  // the suggested ratio 0x80

    SSD1306_SETMULTIPLEX,                  // 0xA8
    SSD1306_LCDHEIGHT - 1,

    SSD1306_SETDISPLAYOFFSET,              // 0xD3
    0x0,                                   // no offset

    SSD1306_SETSTARTLINE | 0x0,            // line #0

    SSD1306_CHARGEPUMP,                    // 0x8D
    0x14,

    SSD1306_MEMORYMODE,                    // 0x20
    0x02,                                  // 0x2 page mode

    SSD1306_SEGREMAP | 0x0,		    // 0xA0

    SSD1306_COMSCANDEC,		    	    // 0xC8

    SSD1306_SETCOMPINS,                    // 0xDA
    0x12,

    SSD1306_SETCONTRAST,                   // 0x81
    0xCF,

    SSD1306_SETPRECHARGE,                  // 0xd9
    0xF1,

    SSD1306_SETVCOMDETECT,                 // 0xDB
    0x40,

    SSD1306_DISPLAYALLON_RESUME,           // 0xA4

    SSD1306_NORMALDISPLAY,                 // 0xA6

    SSD1306_SETPAGESTART,		    // 0xB0
    SSD1306_SETHIGHCOLUMN,		    // 0x10
    SSD1306_SETLOWCOLUMN,	            // 0x00

    SSD1306_DISPLAYON  //--turn on oled     // 0xaf
};

void ssd1306(uint8 i2c, int m)
{
    int i;
    const uint8 *cmds = m ? ssd1306_init_commands_mirror : ssd1306_init_commands;
    mirror = m;
    GFX(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT);
    i2caddr = i2c;

    // Init sequence
    for (i = 0; i < sizeof(ssd1306_init_commands); i++)
        ssd1306_command(cmds[i]);
}

void ssd1306_invertDisplay(uint8 i)
{
    if (i)
        ssd1306_command(SSD1306_INVERTDISPLAY);
    else
        ssd1306_command(SSD1306_NORMALDISPLAY);
}

// ssd1306_scrollRight
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void ssd1306_scrollRight(uint8 start, uint8 stop)
{
    ssd1306_command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
    ssd1306_command(0X00);
    ssd1306_command(start);
    ssd1306_command(0X00);
    ssd1306_command(stop);
    ssd1306_command(0X00);
    ssd1306_command(0XFF);
    ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// ssd1306_scrollLeft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void ssd1306_scrollLeft(uint8 start, uint8 stop)
{
    ssd1306_command(SSD1306_LEFT_HORIZONTAL_SCROLL);
    ssd1306_command(0X00);
    ssd1306_command(start);
    ssd1306_command(0X00);
    ssd1306_command(stop);
    ssd1306_command(0X00);
    ssd1306_command(0XFF);
    ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// ssd1306_scrollDiagRight
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void ssd1306_scrollDiagRight(uint8 start, uint8 stop)
{
    ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);
    ssd1306_command(0X00);
    ssd1306_command(SSD1306_LCDHEIGHT);
    ssd1306_command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
    ssd1306_command(0X00);
    ssd1306_command(start);
    ssd1306_command(0X00);
    ssd1306_command(stop);
    ssd1306_command(0X01);
    ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// ssd1306_scrollDiagLeft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void ssd1306_scrollDiagLeft(uint8 start, uint8 stop)
{
    ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);
    ssd1306_command(0X00);
    ssd1306_command(SSD1306_LCDHEIGHT);
    ssd1306_command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
    ssd1306_command(0X00);
    ssd1306_command(start);
    ssd1306_command(0X00);
    ssd1306_command(stop);
    ssd1306_command(0X01);
    ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void ssd1306_stopScroll(void)
{
    ssd1306_command(SSD1306_DEACTIVATE_SCROLL);
}

// Dim the display
// ssd1306_dim = 1: display is dimmed
// ssd1306_dim = 0: display is normal
void ssd1306_dim(uint8 ssd1306_dim)
{
    uint8 contrast;

    if (ssd1306_dim) {
        contrast = 0; // Dimmed display
    } else
        contrast = 0xCF;
    // the range of contrast to too small to be really useful
    // it is useful to ssd1306_dim the display
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(contrast);
}

void ssd1306_display(void)
{
    uint16 i;
    for (i = 0; i < (SSD1306_LCDHEIGHT / 8); i++)
        ssd1306_displayLine(i);
}

void ssd1306_displayLine(uint8 line)
{
    uint32 i;
    ssd1306_command(SSD1306_SETLOWCOLUMN);
    ssd1306_command(SSD1306_SETHIGHCOLUMN);
    ssd1306_command(SSD1306_SETPAGESTART | (mirror ? (7 - line) : line));

    // send a bunch of data in one xmission
    Oled_begin();
    Oled_write(0x40);
    if (mirror)
        for (i = 0; i < SSD1306_LCDWIDTH; i++)
            Oled_write(ymirror[buffer[(line * SSD1306_LCDWIDTH) + i]]);
    else
        Oled_write_block(buffer + (line * SSD1306_LCDWIDTH), SSD1306_LCDWIDTH);
    Oled_end();
}

// clear everything
void ssd1306_clearDisplay(void)
{
    memset(buffer, 0, sizeof(buffer));
}

void gfx_drawFastHLine(short x, short y, short w, uint16 color)
{
    // Do bounds/limit checks
    if (y < 0 || y >= HEIGHT)
        return;

    // make sure we don't try to draw below 0
    if (x < 0) {
        w += x;
        x = 0;
    }

    // make sure we don't go off the edge of the display
    if ((x + w) > WIDTH)
        w = (WIDTH - x);

    // if our width is now negative, punt
    if (w <= 0)
        return;

    // set up the pointer for  movement through the buffer
    register uint8 *pBuf = buffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * SSD1306_LCDWIDTH);
    // and offset x columns in
    pBuf += x;

    register uint8 mask = 1 << (y & 7);

    switch (color) {
    case WHITE:
        while (w--)
            *pBuf++ |= mask; ;
        break;
    case BLACK:
        mask = ~mask;
        while (w--)
            *pBuf++ &= mask; ;
        break;
    case INVERSE:
        while (w--)
            *pBuf++ ^= mask; ;
        break;
    }
}

void gfx_drawFastVLine(short x, short y, short h, uint16 color)
{

    // do nothing if we're off the left or right side of the screen
    if (x < 0 || x >= WIDTH)
        return;

    // make sure we don't try to draw below 0
    if (y < 0) {
        // y is negative, this will subtract enough from __h to account for __y being 0
        h += y;
        y = 0;

    }

    // make sure we don't go past the height of the display
    if ((y + h) > HEIGHT)
        h = (HEIGHT - y);

    // if our height is now negative, punt
    if (h <= 0)
        return;

    // set up the pointer for fast movement through the buffer
    uint8 *pBuf = buffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * SSD1306_LCDWIDTH);
    // and offset x columns in
    pBuf += x;

    // do the first partial byte, if necessary - this requires some masking
    register uint8 mod = (y & 7);
    if (mod) {
        // mask off the high n bits we want to set
        mod = 8 - mod;

        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        // register uint8 mask = ~(0xFF >> (mod));
        static uint8 premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        register uint8 mask = premask[mod];

        // adjust the mask if we're not going to reach the end of this byte
        if (h < mod)
            mask &= (0XFF >> (mod - h));

        switch (color) {
        case WHITE:
            *pBuf |=  mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^=  mask;
            break;
        }

        // fast exit if we're done here!
        if (h < mod)
            return;

        h -= mod;

        pBuf += SSD1306_LCDWIDTH;
    }


    // write solid bytes while we can - effectively doing 8 rows at a time
    if (h >= 8) {
        if (color ==
            INVERSE)  {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
            do  {
                *pBuf = ~(*pBuf);

                // adjust the buffer forward 8 rows worth of data
                pBuf += SSD1306_LCDWIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
                h -= 8;
            } while (h >= 8);
        } else {
            // store a local value to work with
            register uint8 val = (color == WHITE) ? 255 : 0;

            do  {
                // write our value in
                *pBuf = val;

                // adjust the buffer forward 8 rows worth of data
                pBuf += SSD1306_LCDWIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
                h -= 8;
            } while (h >= 8);
        }
    }

    // now do the final partial byte, if necessary
    if (h) {
        mod = h & 7;
        // this time we want to mask the low bits of the byte, vs the high bits we did above
        // register uint8 mask = (1 << mod) - 1;
        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        static uint8 postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
        register uint8 mask = postmask[mod];
        switch (color) {
        case WHITE:
            *pBuf |=  mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^=  mask;
            break;
        }
    }
}

int ssd1306_checkOled(void)
{
    init_ymirror();
    ssd1306(OLED_I2C_ADDR, 1);
    gfx_writeStr("Starting");
    ssd1306_display();
    return 0;
}


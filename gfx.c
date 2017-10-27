#include "main.h"
#include "glcdfont.h"

short _width, _height, // Display w/h as modified by current rotation
      cursor_x, cursor_y;
uint16 textcolor, textbgcolor;
uint8 textsize;
uint8 wrap;   // If set, 'wrap' text at right edge of display


#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_short
#define _swap_short(a, b) { short t = a; a = b; b = t; }
#endif

void GFX(short w, short h)
{
    _width    = WIDTH;
    _height   = HEIGHT;
    cursor_y  = cursor_x = 0;
    textsize  = 1;
    textcolor = WHITE;
    textbgcolor = BLACK;
    wrap      = 1;
}

// Draw a circle outline
void gfx_drawCircle(short x0, short y0, short r,
                    uint16 color)
{
    short f = 1 - r;
    short ddF_x = 1;
    short ddF_y = -2 * r;
    short x = 0;
    short y = r;

    ssd1306_drawPixel(x0, y0 + r, color);
    ssd1306_drawPixel(x0, y0 - r, color);
    ssd1306_drawPixel(x0 + r, y0, color);
    ssd1306_drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ssd1306_drawPixel(x0 + x, y0 + y, color);
        ssd1306_drawPixel(x0 - x, y0 + y, color);
        ssd1306_drawPixel(x0 + x, y0 - y, color);
        ssd1306_drawPixel(x0 - x, y0 - y, color);
        ssd1306_drawPixel(x0 + y, y0 + x, color);
        ssd1306_drawPixel(x0 - y, y0 + x, color);
        ssd1306_drawPixel(x0 + y, y0 - x, color);
        ssd1306_drawPixel(x0 - y, y0 - x, color);
    }
}

void gfx_drawCircleHelper(short x0, short y0,
                          short r, uint8 cornername, uint16 color)
{
    short f     = 1 - r;
    short ddF_x = 1;
    short ddF_y = -2 * r;
    short x     = 0;
    short y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            ssd1306_drawPixel(x0 + x, y0 + y, color);
            ssd1306_drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            ssd1306_drawPixel(x0 + x, y0 - y, color);
            ssd1306_drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            ssd1306_drawPixel(x0 - y, y0 + x, color);
            ssd1306_drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            ssd1306_drawPixel(x0 - y, y0 - x, color);
            ssd1306_drawPixel(x0 - x, y0 - y, color);
        }
    }
}

void gfx_fillCircle(short x0, short y0, short r,
                    uint16 color)
{
    gfx_drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    gfx_fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void gfx_fillCircleHelper(short x0, short y0, short r,
                          uint8 cornername, short delta, uint16 color)
{

    short f     = 1 - r;
    short ddF_x = 1;
    short ddF_y = -2 * r;
    short x     = 0;
    short y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            gfx_drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            gfx_drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) {
            gfx_drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            gfx_drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}

// Bresenham's algorithm - thx wikpedia
void gfx_drawLine(short x0, short y0, short x1, short y1,
                  uint16 color)
{
    short steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_short(x0, y0);
        _swap_short(x1, y1);
    }

    if (x0 > x1) {
        _swap_short(x0, x1);
        _swap_short(y0, y1);
    }

    short dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    short err = dx / 2;
    short ystep;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (; x0 <= x1; x0++) {
        if (steep)
            ssd1306_drawPixel(y0, x0, color);
        else
            ssd1306_drawPixel(x0, y0, color);
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

// Draw a rectangle
void gfx_drawRect(short x, short y, short w, short h,
                  uint16 color)
{
    gfx_drawFastHLine(x, y, w, color);
    gfx_drawFastHLine(x, y + h - 1, w, color);
    gfx_drawFastVLine(x, y, h, color);
    gfx_drawFastVLine(x + w - 1, y, h, color);
}

void gfx_fillRect(short x, short y, short w, short h,
                  uint16 color)
{
    // Update in subclasses if desired!
    short i;
    for (i = x; i < x + w; i++)
        gfx_drawFastVLine(i, y, h, color);
}

void gfx_fillScreen(uint16 color)
{
    gfx_fillRect(0, 0, _width, _height, color);
}

// Draw a rounded rectangle
void gfx_drawRoundRect(short x, short y, short w,
                       short h, short r, uint16 color)
{
    // smarter version
    gfx_drawFastHLine(x + r, y, w - 2 * r, color);       // Top
    gfx_drawFastHLine(x + r, y + h - 1, w - 2 * r, color);   // Bottom
    gfx_drawFastVLine(x, y + r, h - 2 * r, color);       // Left
    gfx_drawFastVLine(x + w - 1, y + r, h - 2 * r, color);   // Right
    // draw four corners
    gfx_drawCircleHelper(x + r, y + r, r, 1, color);
    gfx_drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    gfx_drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    gfx_drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

// Fill a rounded rectangle
void gfx_fillRoundRect(short x, short y, short w,
                       short h, short r, uint16 color)
{
    // smarter version
    gfx_fillRect(x + r, y, w - 2 * r, h, color);

    // draw four corners
    gfx_fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    gfx_fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

// Draw a triangle
void gfx_drawTriangle(short x0, short y0,
                      short x1, short y1, short x2, short y2, uint16 color)
{
    gfx_drawLine(x0, y0, x1, y1, color);
    gfx_drawLine(x1, y1, x2, y2, color);
    gfx_drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void gfx_fillTriangle(short x0, short y0,
                      short x1, short y1, short x2, short y2, uint16 color)
{

    short a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_short(y0, y1);
        _swap_short(x0, x1);
    }
    if (y1 > y2) {
        _swap_short(y2, y1);
        _swap_short(x2, x1);
    }
    if (y0 > y1) {
        _swap_short(y0, y1);
        _swap_short(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        gfx_drawFastHLine(a, y0, b - a + 1, color);
        return;
    }

    short
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
    int32_t
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1;  // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /*  longhand:
            a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_short(a, b);
        gfx_drawFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /*  longhand:
            a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_short(a, b);
        gfx_drawFastHLine(a, y, b - a + 1, color);
    }
}

// drawBitmap() variant for RAM-resident (not PROGMEM) bitmaps.
void drawBitmap(short x, short y,
                uint8 *bitmap, short w, short h, uint16 color)
{

    short i, j, byteWidth = (w + 7) / 8;
    uint8 byte = 0;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte   = bitmap[j * byteWidth + i / 8];
            if (byte & 0x80)
                ssd1306_drawPixel(x + i, y + j, color);
        }
    }
}

// drawBitmap() variant w/background for RAM-resident (not PROGMEM) bitmaps.
void drawBitmapWithBg(short x, short y,
                      uint8 *bitmap, short w, short h, uint16 color, uint16 bg)
{

    short i, j, byteWidth = (w + 7) / 8;
    uint8 byte = 0;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte   = bitmap[j * byteWidth + i / 8];
            if (byte & 0x80)
                ssd1306_drawPixel(x + i, y + j, color);
            else
                ssd1306_drawPixel(x + i, y + j, bg);
        }
    }
}

void write_ch(uint8 c)
{

    if (c == '\n') {
        cursor_y += textsize * 8;
        cursor_x  = 0;
    } else if (c == '\r') {
        // skip em
    } else {
        if (wrap && ((cursor_x + textsize * 6) >= _width)) { // Heading off edge?
            cursor_x  = 0;            // Reset x to zero
            cursor_y += textsize * 8; // Advance y one line
        }
        gfx_drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
        cursor_x += textsize * 6;
    }
}

void gfx_writeStr(char *buf)
{
    while (*buf)
        write_ch(*buf++);
}

// Draw a character
void gfx_drawChar(short x, short y, unsigned char c,
                  uint16 color, uint16 bg, uint8 size)
{

    if ((x >= _width)            || // Clip right
        (y >= _height)           || // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0))   // Clip top
        return;

    if ((c >= 176))
        c++; // Handle 'classic' charset behavior

    char i;
    for (i = 0; i < 6; i++) {
        uint8 line;
        if (i < 5)
            line = font[(c * 5) + i];
        else
            line = 0x0;
        char j;
        for (j = 0; j < 8; j++, line >>= 1) {
            if (line & 0x1) {
                if (size == 1)
                    ssd1306_drawPixel(x + i, y + j, color);
                else
                    gfx_fillRect(x + (i * size), y + (j * size), size, size, color);
            } else if (bg != color) {
                if (size == 1)
                    ssd1306_drawPixel(x + i, y + j, bg);
                else
                    gfx_fillRect(x + i * size, y + j * size, size, size, bg);
            }
        }
    }
}

void gfx_setCursor(short x, short y)
{
    cursor_x = x;
    cursor_y = y;
}

short getCursorX(void)
{
    return cursor_x;
}

short getCursorY(void)
{
    return cursor_y;
}

void gfx_setTextSize(uint8 s)
{
    textsize = (s > 0) ? s : 1;
}

void gfx_setTextColor(uint16 c)
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

void gfx_setTextColorWithBg(uint16 c, uint16 b)
{
    textcolor   = c;
    textbgcolor = b;
}

void gfx_setTextWrap(uint8 w)
{
    wrap = w;
}

// Pass string and a cursor position, returns UL corner and W,H.
void gfx_getTextBounds(char *str, short x, short y,
                       short *x1, short *y1, uint16 *w, uint16 *h)
{
    uint8 c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    uint16 lineWidth = 0, maxWidth = 0; // Width of current, all lines

    while ((c = *str++)) {
        if (c != '\n') { // Not a newline
            if (c != '\r') { // Not a carriage return, is normal char
                if (wrap && ((x + textsize * 6) >= _width)) {
                    x  = 0;            // Reset x to 0
                    y += textsize * 8; // Advance y by 1 line
                    if (lineWidth > maxWidth)
                        maxWidth = lineWidth; // Save widest line
                    lineWidth  = textsize * 6; // First char on new line
                } else { // No line wrap, just keep incrementing X
                    lineWidth += textsize * 6; // Includes interchar x gap
                }
            } // Carriage return = do nothing
        } else { // Newline
            x  = 0;            // Reset x to 0
            y += textsize * 8; // Advance y by 1 line
            if (lineWidth > maxWidth)
                maxWidth = lineWidth; // Save widest line
            lineWidth = 0;     // Reset lineWidth for new line
        }
    }
    // End of string
    if (lineWidth)
        y += textsize * 8; // Add height of last (or only) line
    if (lineWidth > maxWidth)
        maxWidth = lineWidth; // Is the last or only line the widest?
    *w = maxWidth - 1;               // Don't include last interchar x gap
    *h = y - *y1;

}


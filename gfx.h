#pragma once

#define WIDTH 128
#define HEIGHT 64

void GFX(short w, short h); // Constructor
void gfx_drawLine(short x0, short y0, short x1, short y1, unsigned short color);
void gfx_drawFastVLine(short x, short y, short h, unsigned short color);
void gfx_drawFastHLine(short x, short y, short w, unsigned short color);
void gfx_drawRect(short x, short y, short w, short h, unsigned short color);
void gfx_fillRect(short x, short y, short w, short h, unsigned short color);
void gfx_fillScreen(unsigned short color);
void gfx_drawCircle(short x0, short y0, short r, unsigned short color);
void gfx_drawCircleHelper(short x0, short y0, short r, unsigned char cornername, unsigned short color);
void gfx_fillCircle(short x0, short y0, short r, unsigned short color);
void gfx_fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, unsigned short color);
void gfx_drawTriangle(short x0, short y0, short x1, short y1, short x2, short y2, unsigned short color);
void gfx_fillTriangle(short x0, short y0, short x1, short y1, short x2, short y2, unsigned short color);
void gfx_drawRoundRect(short x0, short y0, short w, short h, short radius, unsigned short color);
void gfx_fillRoundRect(short x0, short y0, short w, short h, short radius, unsigned short color);
void gfx_drawBitmap(short x, short y, const unsigned char *bitmap, short w, short h, unsigned short color);
void gfx_drawBitmapWithBg
(short x, short y, const unsigned char *bitmap, short w, short h, unsigned short color, unsigned short bg);
void gfx_drawChar(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size);
void gfx_setCursor(short x, short y);
void gfx_setTextColor(unsigned short c);
void gfx_setTextColorWithBg(unsigned short c, unsigned short bg);
void gfx_setTextSize(unsigned char s);
void gfx_setTextWrap(unsigned char w);
void gfx_getTextBounds(char *string, short x, short y, short *x2, short *y1, unsigned short *w, unsigned short *h);
void gfx_writeStr(char *buf);

void ssd1306_invertDisplay(unsigned char i);


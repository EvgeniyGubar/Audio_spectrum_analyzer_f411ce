#ifndef _ST7789_H
#define _ST7789_H

#define ST7789_WIDTH	128U
#define ST7789_HEIGHT	160U

#include "main.h"

#define RGB565(r, g, b)         ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)

#define ST7789_ColorMode_65K    0x50
#define ST7789_ColorMode_262K   0x60
#define ST7789_ColorMode_12bit  0x03
#define ST7789_ColorMode_16bit  0x05
#define ST7789_ColorMode_18bit  0x06
#define ST7789_ColorMode_16M    0x07

// Only this COLOR_MODE is emplemented using VRAM
#define COLOR_MODE	ST7789_ColorMode_65K|ST7789_ColorMode_16bit

#define ST7789_Cmd_SWRESET      0x01
#define ST7789_Cmd_SLPIN        0x10
#define ST7789_Cmd_SLPOUT       0x11
#define ST7789_Cmd_COLMOD       0x3A
#define ST7789_Cmd_PTLON        0x12
#define ST7789_Cmd_NORON        0x13
#define ST7789_Cmd_INVOFF       0x20
#define ST7789_Cmd_INVON        0x21
#define ST7789_Cmd_GAMSET       0x26
#define ST7789_Cmd_DISPOFF      0x28
#define ST7789_Cmd_DISPON       0x29
#define ST7789_Cmd_CASET        0x2A
#define ST7789_Cmd_RASET        0x2B
#define ST7789_Cmd_RAMWR        0x2C
#define ST7789_Cmd_PTLAR        0x30
#define ST7789_Cmd_MADCTL       0x36
#define ST7789_Cmd_MADCTL_MY    0x80
#define ST7789_Cmd_MADCTL_MX    0x40
#define ST7789_Cmd_MADCTL_MV    0x20
#define ST7789_Cmd_MADCTL_ML    0x10
#define ST7789_Cmd_MADCTL_RGB   0x00
#define ST7789_Cmd_RDID1        0xDA
#define ST7789_Cmd_RDID2        0xDB
#define ST7789_Cmd_RDID3        0xDC
#define ST7789_Cmd_RDID4        0xDD

#define ST7789_MADCTL_MY        (1<<7)
#define ST7789_MADCTL_MX        (1<<6)
#define ST7789_MADCTL_MV        (1<<5)
#define ST7789_MADCTL_ML        (1<<4)
#define ST7789_MADCTL_BGR       (1<<3)
#define ST7789_MADCTL_MH        (1<<2)

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

typedef enum {
	updateVRAM,		// write element to VRAM without screen update
	updateElement,	// write element to VRAM and show this element only
	updateScreen	// write element to VRAM and update screen according to VRAM
} st7789_Action;

typedef enum {
	OFF, ON
} st7789_Mode;

typedef enum {
	Portrait_0,
	Album_90,
	Portrait_180,
	Album_270,
	Portrait_0_Mirror,
	Album_90_Mirror,
	Portrait_180_Mirror,
	Album_270_Mirror
} st7789_Orientation;

typedef enum {
	Normal,
	Left,
	Right
} st7789_Rotate;

void ST7789_Init(SPI_HandleTypeDef *hspi);
void ST7789_SoftReset(void);
void ST7789_SleepMode(st7789_Mode State);
void ST7789_Orientation(st7789_Orientation State);
void ST7789_ColorModeSet(uint8_t ColorMode);
void ST7789_InversionMode(st7789_Mode State);
void ST7789_SetBL(uint8_t Value);
void ST7789_DisplayPower(st7789_Mode State);

void ST7789_SetWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void ST7789_SavePixel(int16_t x, int16_t y, uint16_t color);
void ST7789_PrintElement(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void ST7789_PrintPixel(int16_t x, int16_t y);
void ST7789_PrintScreen(void);
void ST7789_DrawLine_Slow(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, st7789_Action act);

uint16_t ST7789_RGBToColor(uint8_t r, uint8_t g, uint8_t b);
void ST7789_DrawFillScreen(uint16_t color, st7789_Action act);
void ST7789_DrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, uint8_t draw);
void ST7789_DrawRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, st7789_Action act);
void ST7789_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, st7789_Action act);
void ST7789_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor, st7789_Action act);
void ST7789_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color, st7789_Action act);
void ST7789_DrawChar_5x8(int16_t x, int16_t y, uint16_t TextColor, unsigned char c, st7789_Rotate Rotate, st7789_Action act);
void ST7789_DrawChar_7x11(int16_t x, int16_t y, uint16_t TextColor, unsigned char c, st7789_Rotate Rotate, st7789_Action act);
void ST7789_DrawText_5x8(int16_t x, int16_t y, uint16_t TextColor, char *str, st7789_Rotate Rotate, st7789_Action act);
void ST7789_DrawText_7x11(int16_t x, int16_t y, uint16_t TextColor, char *str, st7789_Rotate Rotate, st7789_Action act);

#endif

#ifndef _ST7789_H
#define _ST7789_H

#define ST7789_WIDTH	128
#define ST7789_HEIGHT	160

#define RES_HIGH	HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_SET)
#define RES_LOW		HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_RESET)
#define CS_HIGH		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define CS_LOW		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define DC_HIGH		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
#define DC_LOW		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)

#include "stm32f4xx_hal.h"

#define RGB565(r, g, b)         ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)

#define ST7789_ColorMode_65K    0x50
#define ST7789_ColorMode_262K   0x60
#define ST7789_ColorMode_12bit  0x03
#define ST7789_ColorMode_16bit  0x05
#define ST7789_ColorMode_18bit  0x06
#define ST7789_ColorMode_16M    0x07

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

#define ST7789_MADCTL_MY        7
#define ST7789_MADCTL_MX        6
#define ST7789_MADCTL_MV        5
#define ST7789_MADCTL_ML        0x10
#define ST7789_MADCTL_BGR       0x08
#define ST7789_MADCTL_MH        0x04

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

// Смещение матрицы относительно строк/столбцов контроллера
#define ST7789_X_Start          0
#define ST7789_Y_Start          0

void ST7789_Init(SPI_HandleTypeDef *hspi);
void ST7789_HardReset(void);
void ST7789_SoftReset(void);
void ST7789_SleepModeEnter(void);
void ST7789_SleepModeExit(void);
void ST7789_ColorModeSet(uint8_t ColorMode);
void ST7789_MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror, uint8_t HorizMirror);
void ST7789_InversionMode(uint8_t Mode);
void ST7789_FillScreen(uint16_t color);
void ST7789_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, uint8_t draw);
void ST7789_SetWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void ST7789_RamWrite(uint16_t *pBuff, uint16_t Len);

void ST7789_DrawLine_for_Analyzer(int16_t y, int16_t x_max);
void ST7789_DrawColumn_for_Analyzer(int16_t y, int16_t x_max);

void ST7789_SendFrame();
void ST7789_SetBL(uint8_t Value);
void ST7789_DisplayPower(uint8_t On);
void ST7789_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void ST7789_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor);
void ST7789_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t draw);
void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t *color);
void ST7789_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor);
void ST7789_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color);
void ST7789_DrawChar_5x8(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, unsigned char c);
void ST7789_DrawChar_7x11(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, unsigned char c);
void ST7789_print_5x8(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str);
void ST7789_print_7x11(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str);
uint16_t ST7789_RGBToColor(uint8_t r, uint8_t g, uint8_t b);

#endif

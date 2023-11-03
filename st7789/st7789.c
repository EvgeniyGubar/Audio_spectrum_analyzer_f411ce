#include "st7789.h"
#include "st7789_spi.h"
#include "font5x7.h"
#include "font7x11.h"
#include <stdlib.h>
#include <string.h>

extern SPI_HandleTypeDef *hspi_lcd;

uint16_t lcd_X_max, lcd_Y_max;

uint16_t VRAM[ST7789_WIDTH * ST7789_HEIGHT] =
{ 0 };

/**********************************************************************/
void ST7789_Init(SPI_HandleTypeDef *hspi)
{
	hspi_lcd = hspi;
	ST7789_SoftReset();
	ST7789_SleepMode(OFF);
	ST7789_ColorModeSet(COLOR_MODE);
	ST7789_Orientation(Portrait_180);
	ST7789_InversionMode(OFF);
	ST7789_DrawFillScreen(BLACK, updateScreen);
	ST7789_SetBL(100);
	ST7789_DisplayPower(ON);
}

/**********************************************************************/
void ST7789_SoftReset(void)
{
	ST7789_SendCmd(ST7789_Cmd_SWRESET);
	HAL_Delay(120);
}
/**********************************************************************/
void ST7789_SleepMode(st7789_Mode State)
{
	State == ON ? ST7789_SendCmd(ST7789_Cmd_SLPIN) : ST7789_SendCmd(ST7789_Cmd_SLPOUT);
	HAL_Delay(120);
}
/**********************************************************************/
void ST7789_DisplayPower(st7789_Mode State)
{
	State == ON ? ST7789_SendCmd(ST7789_Cmd_DISPON) : ST7789_SendCmd(ST7789_Cmd_DISPOFF);
}
/**********************************************************************/
void ST7789_ColorModeSet(uint8_t ColorMode)
{
	ST7789_SendCmd(ST7789_Cmd_COLMOD);
	ST7789_SendData(ColorMode);
}
/**********************************************************************/
void ST7789_InversionMode(st7789_Mode State)
{
	State == ON ? ST7789_SendCmd(ST7789_Cmd_INVON) : ST7789_SendCmd(ST7789_Cmd_INVOFF);
}
/**********************************************************************/
void ST7789_Orientation(st7789_Orientation State)
{
	uint8_t data = 0;
	ST7789_SendCmd(ST7789_Cmd_MADCTL);

	switch (State)
	{
	case Portrait_0:
		{
			data = 0;
			lcd_Y_max = ST7789_HEIGHT;
			lcd_X_max = ST7789_WIDTH;
		}
		break;
	case Album_90:
		{
			data = ST7789_MADCTL_MV | ST7789_MADCTL_MX;
			lcd_Y_max = ST7789_WIDTH;
			lcd_X_max = ST7789_HEIGHT;
		}
		break;
	case Portrait_180:
		{
			data = ST7789_MADCTL_MX | ST7789_MADCTL_MY;
			lcd_Y_max = ST7789_HEIGHT;
			lcd_X_max = ST7789_WIDTH;
		}
		break;
	case Album_270:
		{
			data = ST7789_MADCTL_MV | ST7789_MADCTL_MY;
			lcd_Y_max = ST7789_WIDTH;
			lcd_X_max = ST7789_HEIGHT;
		}
		break;
	case Portrait_0_Mirror:
		{
			data = ST7789_MADCTL_MX;
			lcd_Y_max = ST7789_HEIGHT;
			lcd_X_max = ST7789_WIDTH;
		}
		break;
	case Album_90_Mirror:
		{
			data = ST7789_MADCTL_MV | ST7789_MADCTL_MX | ST7789_MADCTL_MY;
			lcd_Y_max = ST7789_WIDTH;
			lcd_X_max = ST7789_HEIGHT;
		}
		break;
	case Portrait_180_Mirror:
		{
			data = ST7789_MADCTL_MY;
			lcd_Y_max = ST7789_HEIGHT;
			lcd_X_max = ST7789_WIDTH;
		}
		break;
	case Album_270_Mirror:
		{
			data = ST7789_MADCTL_MV;
			lcd_Y_max = ST7789_WIDTH;
			lcd_X_max = ST7789_HEIGHT;
		}
		break;
	default:
		break;
	}
	ST7789_SendData(data);
}

/**********************************************************************/
void ST7789_SetWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	if ((x0 > x1) || (x1 > lcd_X_max) || (y0 > y1) || (y1 > lcd_Y_max)) return;

	uint16_t data[8];

	ST7789_SendCmd(ST7789_Cmd_CASET);
	data[0] = x0;
	data[1] = x1;
	ST7789_SendData_Block((uint8_t*) &data[0], 2);

	ST7789_SendCmd(ST7789_Cmd_RASET);
	data[2] = y0;
	data[3] = y1;
	ST7789_SendData_Block((uint8_t*) &data[2], 2);

	ST7789_SendCmd(ST7789_Cmd_RAMWR);
}

/**********************************************************************/
void ST7789_SavePixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= lcd_X_max) || (y < 0) || (y >= lcd_Y_max)) return;
	VRAM[x + lcd_X_max * y] = color;
}

/**********************************************************************/
void ST7789_PrintElement(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	if (x0 < 0) x0 = 0;
	if (x1 >= lcd_X_max) x1 = lcd_X_max;
	if (y0 < 0) y0 = 0;
	if (y1 >= lcd_Y_max) y1 = lcd_Y_max;

	for (int16_t j = y0; j < y1; ++j)
	{
		ST7789_SetWindow(x0, j, x1, j);
		ST7789_SendData_Block((uint8_t*) &VRAM[x0 + lcd_X_max * j], (x1 - x0));
	}
}
/**********************************************************************/
void ST7789_PrintPixel(int16_t x, int16_t y)
{
	if ((x < 0) || (x >= lcd_X_max) || (y < 0) || (y >= lcd_Y_max)) return;
	ST7789_SetWindow(x, y, x, y);
	ST7789_SendData_Block((uint8_t*) &VRAM[x + lcd_X_max * y], 1);
}

/**********************************************************************/
void ST7789_PrintScreen(void)
{
	ST7789_SetWindow(0, 0, lcd_X_max, lcd_Y_max);
	ST7789_SendData_Block((uint8_t*) &VRAM[0], ST7789_WIDTH * ST7789_HEIGHT);
}

/**********************************************************************
 *		    High level functions for drawing primitives
 *********************************************************************/
void ST7789_DrawFillScreen(uint16_t color, st7789_Action act)
{
	ST7789_DrawFillRect(0, 0, lcd_X_max, lcd_Y_max, color, act);
}

/**********************************************************************/
void ST7789_DrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, st7789_Action act)
{
	if ((x >= lcd_X_max) || (y >= lcd_Y_max)) return;
	if ((x + w) > lcd_X_max) w = lcd_X_max - x;
	if ((y + h) > lcd_Y_max) h = lcd_Y_max - y;

	for (int16_t j = y; j < y + h; ++j)
	{
		for (int16_t i = x; i < x + w; ++i)
		{
			ST7789_SavePixel(i, j, color);
		}
	}

	if (act == updateElement) ST7789_PrintElement(x, y, x + w, y + h);
	else if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawLine_Slow(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, st7789_Action act)
{
	const int16_t deltaX = abs(x1 - x0);
	const int16_t deltaY = abs(y1 - y0);
	const int16_t signX = x0 < x1 ? 1 : -1;
	const int16_t signY = y0 < y1 ? 1 : -1;
	int16_t error = deltaX - deltaY;
	int16_t x = x0;
	int16_t y = y0;

	do
	{
		int16_t error2 = error * 2;

		if (error2 > -deltaY)
		{
			error -= deltaY;
			x += signX;
		}
		if (error2 < deltaX)
		{
			error += deltaX;
			y += signY;
		}
		ST7789_SavePixel(x, y, color);

		if (act == updateElement) ST7789_PrintPixel(x, y);

	} while (x != x1 || y != y1);

	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, st7789_Action act)
{
// Horizontal line
	if (x0 == x1)
	{
		if (y0 > y1) ST7789_DrawFillRect(x0, y1, 1, y0 - y1 + 1, color, act);
		else ST7789_DrawFillRect(x0, y0, 1, y1 - y0 + 1, color, act);
		return;
	}

// Vertical line
	if (y0 == y1)
	{
		if (x0 > x1) ST7789_DrawFillRect(x1, y0, x0 - x1 + 1, 1, color, act);
		else ST7789_DrawFillRect(x0, y0, x1 - x0 + 1, 1, color, act);
		return;
	}

	ST7789_DrawLine_Slow(x0, y0, x1, y1, color, act);
}

/**********************************************************************/
void ST7789_DrawRect(int16_t x, int16_t y, int16_t h, int16_t w, uint16_t color, st7789_Action act)
{
	ST7789_DrawLine(x, y, x, y + w, color, act);
	ST7789_DrawLine(x + h, y, x + h, y + w, color, act);
	ST7789_DrawLine(x, y, x + h, y, color, act);
	ST7789_DrawLine(x, y + w, x + h, y + w, color, act);
}
/**********************************************************************/
void ST7789_DrawRectCoordinates(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, st7789_Action act)
{
	ST7789_DrawLine(x0, y0, x0, y1, color, act);
	ST7789_DrawLine(x1, y0, x1, y1, color, act);
	ST7789_DrawLine(x0, y0, x1, y0, color, act);
	ST7789_DrawLine(x0, y1, x1, y1, color, act);
}

/**********************************************************************/
void ST7789_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t color, st7789_Action act)
{
	int16_t x = 0;
	int16_t y = radius;
	int16_t dp = 1 - radius;
	do
	{
		if (act != updateScreen)
		{
			ST7789_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color, act);
			ST7789_DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color, act);
			ST7789_DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color, act);
			ST7789_DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color, act);
		}
		else
		{
			ST7789_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color, updateVRAM);
			ST7789_DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color, updateVRAM);

			ST7789_DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color, updateVRAM);
			ST7789_DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color, updateVRAM);
		}

		if (dp < 0)
		{
			dp += (2 * x) + 1;
		}
		else
		{
			y = y - 1;
			dp += (2 * x) - (2 * y) + 1;
		}
		x = x + 1;
	} while (y >= x);

	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color, st7789_Action act)
{
	int16_t x = 0;
	int16_t y = radius;
	int16_t dp = 1 - radius;
	do
	{
		ST7789_SavePixel(x0 + x, y0 + y, color);
		ST7789_SavePixel(x0 + y, y0 + x, color);
		ST7789_SavePixel(x0 - y, y0 + x, color);
		ST7789_SavePixel(x0 - x, y0 + y, color);
		ST7789_SavePixel(x0 - x, y0 - y, color);
		ST7789_SavePixel(x0 - y, y0 - x, color);
		ST7789_SavePixel(x0 + y, y0 - x, color);
		ST7789_SavePixel(x0 + x, y0 - y, color);

		if (act == updateElement)
		{
			ST7789_PrintPixel(x0 + x, y0 + y);
			ST7789_PrintPixel(x0 + y, y0 + x);
			ST7789_PrintPixel(x0 - y, y0 + x);
			ST7789_PrintPixel(x0 - x, y0 + y);
			ST7789_PrintPixel(x0 - x, y0 - y);
			ST7789_PrintPixel(x0 - y, y0 - x);
			ST7789_PrintPixel(x0 + y, y0 - x);
			ST7789_PrintPixel(x0 + x, y0 - y);
		}

		if (dp < 0)
		{
			dp += (2 * x) + 1;
		}
		else
		{
			y = y - 1;
			dp += (2 * x) - (2 * y) + 1;
		}
		x = x + 1;
	} while (y >= x);

	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawChar_5x8(int16_t x, int16_t y, uint16_t TextColor, unsigned char c, st7789_Rotate Rotate, st7789_Action act, uint16_t backgroundColor)
{
//	if ((x >= lcd_X_max) || (y >= lcd_Y_max) || ((x + 4) < 0) || ((y + 7) < 0)) return;

	if (c < 128) c = c - 32;
	if (c >= 144 && c <= 175) c = c - 48;
	if (c >= 128 && c <= 143) c = c + 16;
	if (c >= 176 && c <= 191) c = c - 48;
	if (c > 191) return;

	for (uint8_t i = 0; i < 6; i++)
	{
		uint8_t coloumn;

		if (i == 5) coloumn = 0x00;
		else coloumn = font[(c * 5) + i];

		for (uint8_t j = 0; j < 8; j++)
		{
			switch (Rotate)
			{
			case Normal:
				{
					ST7789_SavePixel(x + i, y + j, (coloumn & 0x01) ? TextColor : backgroundColor);
				}
				break;

			case Left:
				{

				}
				break;
			case Right:
				{
					ST7789_SavePixel(lcd_X_max - (y + j), x + i, (coloumn & 0x01) ? TextColor : backgroundColor);
				}
				break;
			}
			coloumn >>= 1;
		}
	}
	if (act == updateElement) ST7789_PrintElement(x, y, x + 5, y + 8);
	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawChar_7x11(int16_t x, int16_t y, uint16_t TextColor, unsigned char c, st7789_Rotate Rotate, st7789_Action act, uint16_t backgroundColor)
{
	uint8_t buffer[11];

//	if ((x >= lcd_X_max) || (y >= lcd_Y_max) || ((x + 7) < 0) || ((y + 11) < 0)) return;

	if (c < 128) c = c - 32;
	if (c >= 144 && c <= 175) c = c - 48;
	if (c >= 128 && c <= 143) c = c + 16;
	if (c >= 176 && c <= 191) c = c - 48;
	if (c > 191) return;

// Copy selected simbol to buffer
	memcpy(buffer, &font7x11[c * 11], 11);

	for (uint8_t j = 0; j < 11; j++)
	{
		for (uint8_t i = 0; i < 7; i++)
		{
			switch (Rotate)
			{
			case Normal:
				{
					ST7789_SavePixel(x + i, y + j, (buffer[j] & (1 << i)) ? TextColor : backgroundColor);
					break;
				}

			case Left:
				{

					break;
				}
			case Right:
				{
					ST7789_SavePixel(lcd_X_max - (y + j), x + i, (buffer[j] & (1 << i)) ? TextColor : backgroundColor);
				}
				break;
			}
		}
	}
	if (act == updateElement) ST7789_PrintElement(x, y, x + 7, y + 11);
	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawText_5x8(int16_t x, int16_t y, uint16_t TextColor, char *str, st7789_Rotate Rotate, st7789_Action act, uint16_t backgroundColor)
{
	int16_t x0 = x, y0 = y;
	unsigned char type = *str;
	uint8_t count = 0;
	if (type >= 128) x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_5x8(x, y, TextColor, *str++, Rotate, updateVRAM, backgroundColor);
		type = *str;
		if (type >= 128) x = x + 3;
		else x = x + 6;
		++count;
	}
	if (act == updateElement) ST7789_PrintElement(x0, y0, x, y + 8);
	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
void ST7789_DrawText_7x11(int16_t x, int16_t y, uint16_t TextColor, char *str, st7789_Rotate Rotate, st7789_Action act, uint16_t backgroundColor)
{
	int16_t x0 = x, y0 = y;
	unsigned char type = *str;
	uint8_t count = 0;
	if (type >= 128) x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_7x11(x, y, TextColor, *str++, Rotate, updateVRAM, backgroundColor);
		type = *str;
		if (type >= 128) x = x + 4;
		else x = x + 8;
		++count;
	}
	if (act == updateElement) ST7789_PrintElement(x0, y0, x, y + 11);
	if (act == updateScreen) ST7789_PrintScreen();
}

/**********************************************************************/
uint16_t ST7789_RGBToColor(uint8_t r, uint8_t g, uint8_t b)
{
	return (((uint16_t) r >> 3) << 3) | (((uint16_t) b >> 3) << 8) | (((uint16_t) g >> 2) << 13) | ((uint16_t) g >> 5);
}

/**********************************************************************/
void ST7789_SetBL(uint8_t Value)
{
	if (Value > 100) Value = 100;

#if (ST77xx_BLK_PWM_Used)
  //tmr2_PWM_set(ST77xx_PWM_TMR2_Chan, Value);
//#else
//	if (Value)
//		HAL_GPIO_WritePin(BLK_GPIO_Port, BLK_Pin, GPIO_PIN_SET);
//	else
//		HAL_GPIO_WritePin(BLK_GPIO_Port, BLK_Pin, GPIO_PIN_RESET);
#endif
}

/**********************************************************************/
//static void hsvToRgb(uint16_t h, uint8_t *result)
//{
//	uint8_t red, green, blue;
//	uint8_t f = (h % 60) * 255 / 60;
//	uint8_t p = 0;
//	uint8_t q = (255 - f ) * value / 255;
//	uint8_t t = (255 - (255 - f)) * value / 255;
//
//	switch((h / 60) % 6)
//	{
//		case 0: red = value; green = t; blue = p; break;
//		case 1: red = q; green = value; blue = p; break;
//		case 2: red = p; green = value; blue = t; break;
//		case 3: red = p; green = q; blue = value; break;
//		case 4: red = t; green = p; blue = value; break;
//		case 5: red = value; green = p; blue = q; break;
//	}
//	result[0] = green;
//	result[1] = red;
//	result[2] = blue;
//}

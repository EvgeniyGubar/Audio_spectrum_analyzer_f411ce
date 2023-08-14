#include "st7789.h"
#include "main.h"
#include "font5x7.h"
#include "font7x11.h"
#include <stdlib.h>
#include <string.h>

SPI_HandleTypeDef *hspi_lcd;

static void ST7789_SendCmd(uint8_t Cmd);
static void ST7789_SendData(uint8_t Data);
//static void ST7789_ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd);
//static void ST7789_RowSet(uint16_t RowStart, uint16_t RowEnd);
static void ST7789_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2);
uint8_t flag_end_tx = 0;

uint8_t VRAM[ST7789_WIDTH * ST7789_HEIGHT * 2] = { 0 };

/**********************************************************************/
void ST7789_Init(SPI_HandleTypeDef *hspi)
{
	hspi_lcd = hspi;
	ST7789_HardReset();
	ST7789_SoftReset();
	ST7789_SleepModeExit();

	ST7789_ColorModeSet(ST7789_ColorMode_65K | ST7789_ColorMode_16bit);
	HAL_Delay(1);
	ST7789_MemAccessModeSet(0, 1, 1);
	HAL_Delay(1);
	ST7789_InversionMode(0);
	HAL_Delay(1);
	ST7789_FillScreen(BLACK);
	HAL_Delay(14);		// длительность зависит от скорости заливки экрана. на 25 мбит/с это 14 мс
	ST7789_SetBL(10);
	ST7789_DisplayPower(1);
	HAL_Delay(1);
}

/**********************************************************************/
void ST7789_HardReset(void)
{
	RES_LOW;
	HAL_Delay(1);
	RES_HIGH;
	HAL_Delay(120);
}

/**********************************************************************/
void ST7789_SoftReset(void)
{
	ST7789_SendCmd(ST7789_Cmd_SWRESET);
	HAL_Delay(130);
}

/**********************************************************************/
void ST7789_SleepModeEnter(void)
{
	ST7789_SendCmd(ST7789_Cmd_SLPIN);
	HAL_Delay(500);
}

/**********************************************************************/
void ST7789_SleepModeExit(void)
{
	ST7789_SendCmd(ST7789_Cmd_SLPOUT);
	HAL_Delay(120);
}

/**********************************************************************/
void ST7789_DisplayPower(uint8_t On)
{
	On ? ST7789_SendCmd(ST7789_Cmd_DISPON) : ST7789_SendCmd(ST7789_Cmd_DISPOFF);
}

/**********************************************************************/
void ST7789_ColorModeSet(uint8_t ColorMode)
{
	ST7789_SendCmd(ST7789_Cmd_COLMOD);
	ST7789_SendData(ColorMode & 0x77);
}

/**********************************************************************/
void ST7789_MemAccessModeSet(uint8_t Exchange_XY, uint8_t Mirror_X, uint8_t Mirror_Y)
{
	ST7789_SendCmd(ST7789_Cmd_MADCTL);
	ST7789_SendData((Exchange_XY << ST7789_MADCTL_MV) | (Mirror_X << ST7789_MADCTL_MX)
			| (Mirror_Y << ST7789_MADCTL_MY));
}

/**********************************************************************/
void ST7789_InversionMode(uint8_t Mode)
{
	Mode ? ST7789_SendCmd(ST7789_Cmd_INVON) : ST7789_SendCmd(ST7789_Cmd_INVOFF);
}

/**********************************************************************/
void ST7789_SetWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	if (x0 > x1) return;
	if (x1 > ST7789_WIDTH) return;
	if (y0 > y1) return;
	if (y1 > ST7789_HEIGHT) return;

//	ColumnStart += ST7789_X_Start;
//	ColumnEnd += ST7789_X_Start;
	ST7789_SendCmd(ST7789_Cmd_CASET);
	ST7789_SendData(x0 >> 8);
	ST7789_SendData(x0 & 0xFF);
	ST7789_SendData(x1 >> 8);
	ST7789_SendData(x1 & 0xFF);

//	RowStart += ST7789_Y_Start;
//	RowEnd += ST7789_Y_Start;
	ST7789_SendCmd(ST7789_Cmd_RASET);
	ST7789_SendData(y0 >> 8);
	ST7789_SendData(y0 & 0xFF);
	ST7789_SendData(y1 >> 8);
	ST7789_SendData(y1 & 0xFF);

	ST7789_SendCmd(ST7789_Cmd_RAMWR);
}

/**********************************************************************/
static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2)
{
	int16_t TempValue = *pValue1;
	*pValue1 = *pValue2;
	*pValue2 = TempValue;
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
//void ST7789_RamWrite(uint16_t *pBuff, uint16_t Len)
//{
//	while (Len--)
//	{
//		ST7789_SendData(*pBuff >> 8);
//		ST7789_SendData(*pBuff & 0xFF);
//	}
//}
/**********************************************************************/
static void ST7789_SendCmd(uint8_t Cmd)
{
	while (flag_end_tx);
	DC_LOW;
	CS_LOW;
//	if (READ_BIT(SPI1->CR1, SPI_CR1_SPE) != (SPI_CR1_SPE))
//		SET_BIT(SPI1->CR1, SPI_CR1_SPE);
//	SPI1->DR = Cmd;
//	while (!(SPI1->SR & SPI_SR_TXE));
	flag_end_tx = 1;
	HAL_SPI_Transmit_IT(hspi_lcd, &Cmd, 1);
//	CS_HIGH;
}

/**********************************************************************/
static void ST7789_SendData(uint8_t Data)
{
	while (flag_end_tx);
	DC_HIGH;
	CS_LOW;
//	if (READ_BIT(SPI2->CR1, SPI_CR1_SPE) != (SPI_CR1_SPE))
//		SET_BIT(SPI2->CR1, SPI_CR1_SPE);
//	SPI2->DR = Data;
//	while (!(SPI2->SR & SPI_SR_TXE));
	flag_end_tx = 1;
	HAL_SPI_Transmit_IT(hspi_lcd, &Data, 1);
//	CS_HIGH;
}

/**********************************************************************/
void ST7789_SendFrame(void)
{
	while (flag_end_tx);
	ST7789_SetWindow(0, 0, ST7789_WIDTH, ST7789_HEIGHT);
	DC_HIGH;
	CS_LOW;
	flag_end_tx = 1;
	HAL_SPI_Transmit_DMA(hspi_lcd, VRAM, ST7789_WIDTH * ST7789_HEIGHT * 2);
}

/**********************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	CS_HIGH;
	flag_end_tx = 0;
}

/**********************************************************************/
void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t *color)
{
	if ((x < 0) || (x >= ST7789_WIDTH) || (y < 0) || (y >= ST7789_HEIGHT)) return;

	VRAM[(x + ST7789_WIDTH * y) * 2] = *color >> 8;
	VRAM[(x + ST7789_WIDTH * y) * 2 + 1] = *color & 0xFF;
}

/**********************************************************************
 *
 * 				Графические функции высокого уровня
 *
 *********************************************************************/
void ST7789_FillScreen(uint16_t color)
{
	ST7789_FillRect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color, 1);
}

/**********************************************************************/
void ST7789_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, uint8_t draw)
{
	if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
	if ((x + w) > ST7789_WIDTH) w = ST7789_WIDTH - x;
	if ((y + h) > ST7789_HEIGHT) h = ST7789_HEIGHT - y;

	for (int16_t i = x; i < x + w; ++i)
	{
		for (int16_t j = y; j < y + h; ++j)
		{
			ST7789_DrawPixel(i, j, &color);
		}
	}

//	for (uint32_t i = 0; i < (h * w * 2); i += 2)
//	{
//		VRAM[i] = color >> 8;
//		VRAM[i + 1] = color & 0xFF;
//	}

	if (draw)
	{
		ST7789_SendFrame();

	}
}

/**********************************************************************/
void ST7789_DrawLine_for_Analyzer(int16_t y, int16_t x_max)
{
	if (x_max > 128)
	{
		x_max = 128;
	}

	uint16_t offset = y * ST7789_WIDTH * 2;
	uint16_t offset1 = (y + 1) * ST7789_WIDTH * 2;
	uint16_t offset2 = (y + 2) * ST7789_WIDTH * 2;
	uint16_t offset3 = (y + 3) * ST7789_WIDTH * 2;

	for (uint16_t i = 0; i < x_max * 2; i += 2)
	{
		VRAM[i + offset] = BLUE >> 8;
		VRAM[i + 1 + offset] = BLUE & 0xFF;
		VRAM[i + offset1] = BLUE >> 8;
		VRAM[i + 1 + offset1] = BLUE & 0xFF;
		VRAM[i + offset2] = BLUE >> 8;
		VRAM[i + 1 + offset2] = BLUE & 0xFF;
		VRAM[i + offset3] = BLUE >> 8;
		VRAM[i + 1 + offset3] = BLUE & 0xFF;
	}
	for (uint16_t i = x_max * 2; i < ST7789_WIDTH * 2; i += 2)
	{
		VRAM[i + offset] = BLACK >> 8;
		VRAM[i + 1 + offset] = BLACK & 0xFF;
		VRAM[i + offset1] = BLACK >> 8;
		VRAM[i + 1 + offset1] = BLACK & 0xFF;
		VRAM[i + offset2] = BLACK >> 8;
		VRAM[i + 1 + offset2] = BLACK & 0xFF;
		VRAM[i + offset3] = BLACK >> 8;
		VRAM[i + 1 + offset3] = BLACK & 0xFF;
	}
}

/**********************************************************************/
void ST7789_DrawColumn_for_Analyzer(int16_t y, int16_t x_max)
{
	if (x_max > 128)
	{
		x_max = 128;
	}
	//in develop
}

/**********************************************************************/
void ST7789_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	ST7789_DrawLine(x1, y1, x1, y2, color, 0);
	ST7789_DrawLine(x2, y1, x2, y2, color, 0);
	ST7789_DrawLine(x1, y1, x2, y1, color, 0);
	ST7789_DrawLine(x1, y2, x2, y2, color, 0);
	ST7789_SendFrame();
}

/**********************************************************************/
void ST7789_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor)
{
	if (x1 > x2) SwapInt16Values(&x1, &x2);
	if (y1 > y2) SwapInt16Values(&y1, &y2);
	ST7789_FillRect(x1, y1, x2 - x1, y2 - y1, fillcolor, 1);
}

/**********************************************************************/
void ST7789_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t draw)
{
	// Вертикальная линия
	if (x1 == x2)
	{
		// Отрисовываем линию быстрым методом
		if (y1 > y2) ST7789_FillRect(x1, y2, 1, y1 - y2 + 1, color, 0);
		else ST7789_FillRect(x1, y1, 1, y2 - y1 + 1, color, 0);
		return;
	}

	// Горизонтальная линия
	if (y1 == y2)
	{
		// Отрисовываем линию быстрым методом
		if (x1 > x2) ST7789_FillRect(x2, y1, x1 - x2 + 1, 1, color, 0);
		else ST7789_FillRect(x1, y1, x2 - x1 + 1, 1, color, 0);
		return;
	}

	ST7789_DrawLine_Slow(x1, y1, x2, y2, color);
	if (draw)
	{
		ST7789_SendFrame();
	}
}

/**********************************************************************/
static void ST7789_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	const int16_t deltaX = abs(x2 - x1);
	const int16_t deltaY = abs(y2 - y1);
	const int16_t signX = x1 < x2 ? 1 : -1;
	const int16_t signY = y1 < y2 ? 1 : -1;

	int16_t error = deltaX - deltaY;

	ST7789_DrawPixel(x2, y2, &color);

	while (x1 != x2 || y1 != y2)
	{
		ST7789_DrawPixel(x1, y1, &color);
		const int16_t error2 = error * 2;

		if (error2 > -deltaY)
		{
			error -= deltaY;
			x1 += signX;
		}
		if (error2 < deltaX)
		{
			error += deltaX;
			y1 += signY;
		}
	}
}

/**********************************************************************/
//void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color)
//{
//	if ((x < 0) || (x >= ST7789_WIDTH) || (y < 0) || (y >= ST7789_HEIGHT)) return;
//
//	ST7789_DrawPixel(x, y, &color);
//	ST7789_SetWindow(x, y, x, y);
//	ST7789_RamWrite(&color, 1);
//}
/**********************************************************************/
void ST7789_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor)
{
	int x = 0;
	int y = radius;
	int delta = 1 - 2 * radius;
	int error = 0;

	while (y >= 0)
	{
		ST7789_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor, 0);
		ST7789_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor, 0);
		error = 2 * (delta + y) - 1;

		if (delta < 0 && error <= 0)
		{
			++x;
			delta += 2 * x + 1;
			continue;
		}

		error = 2 * (delta - x) - 1;

		if (delta > 0 && error > 0)
		{
			--y;
			delta += 1 - 2 * y;
			continue;
		}

		++x;
		delta += 2 * (x - y);
		--y;
	}
	ST7789_SendFrame();
}

/**********************************************************************/
void ST7789_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color)
{
	int x = 0;
	int y = radius;
	int delta = 1 - 2 * radius;
	int error = 0;

	while (y >= 0)
	{
		ST7789_DrawPixel(x0 + x, y0 + y, &color);
		ST7789_DrawPixel(x0 + x, y0 - y, &color);
		ST7789_DrawPixel(x0 - x, y0 + y, &color);
		ST7789_DrawPixel(x0 - x, y0 - y, &color);
		error = 2 * (delta + y) - 1;

		if (delta < 0 && error <= 0)
		{
			++x;
			delta += 2 * x + 1;
			continue;
		}

		error = 2 * (delta - x) - 1;

		if (delta > 0 && error > 0)
		{
			--y;
			delta += 1 - 2 * y;
			continue;
		}

		++x;
		delta += 2 * (x - y);
		--y;
	}
	ST7789_SendFrame();
}

/**********************************************************************/
void ST7789_DrawChar_5x8(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	if ((x >= 240) || (y >= 240) || ((x + 4) < 0) || ((y + 7) < 0)) return;
	if (c < 128) c = c - 32;
	if (c >= 144 && c <= 175) c = c - 48;
	if (c >= 128 && c <= 143) c = c + 16;
	if (c >= 176 && c <= 191) c = c - 48;
	if (c > 191) return;
	for (uint8_t i = 0; i < 6; i++)
	{
		uint8_t line;
		if (i == 5) line = 0x00;
		else line = font[(c * 5) + i];
		for (uint8_t j = 0; j < 8; j++)
		{
			if (line & 0x01) ST7789_DrawPixel(x + i, y + j, &TextColor);
			else if (!TransparentBg) ST7789_DrawPixel(x + i, y + j, &BgColor);
			line >>= 1;
		}
	}
//	ST7789_SendFrame();
}

/**********************************************************************/
void ST7789_DrawChar_7x11(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j;
	uint8_t buffer[11];

	if ((x >= 240) || (y >= 240) || ((x + 4) < 0) || ((y + 7) < 0)) return;

	// Copy selected simbol to buffer
	memcpy(buffer, &font7x11[(c - 32) * 11], 11);
	for (j = 0; j < 11; j++)
	{
		for (i = 0; i < 7; i++)
		{
			if ((buffer[j] & (1 << i)) == 0)
			{
				if (!TransparentBg) ST7789_DrawPixel(x + i, y + j, &BgColor);
			}
			else ST7789_DrawPixel(x + i, y + j, &TextColor);
		}
	}
//	ST7789_SendFrame();
}

/**********************************************************************/
void ST7789_print_5x8(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128) x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_5x8(x, y, TextColor, BgColor, TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128) x = x + 3;
		else x = x + 6;
	}
	ST7789_SendFrame();
}

/**********************************************************************/
void ST7789_print_7x11(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128) x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_7x11(x, y, TextColor, BgColor, TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128) x = x + 8;
		else x = x + 8;
	}

	ST7789_SendFrame();
}

/**********************************************************************/
uint16_t ST7789_RGBToColor(uint8_t r, uint8_t g, uint8_t b)
{
	return (((uint16_t) r >> 3) << 3) | (((uint16_t) b >> 3) << 8) | (((uint16_t) g >> 2) << 13) | ((uint16_t) g >> 5);
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

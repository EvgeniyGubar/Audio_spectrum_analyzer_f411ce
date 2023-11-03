/*
 * lcd_analyzer.h
 *
 *  Created on: Aug 29, 2023
 *      Author: Evgeniy
 */

#ifndef LCD_ANALYZER_H_
#define LCD_ANALYZER_H_

#include "st7789.h"

extern uint16_t VRAM[];

/**********************************************************************/
void ST7789_DrawLine_for_Analyzer(int16_t column, int16_t value)
{
	uint16_t offset = column * ST7789_WIDTH;

	if (value > ST7789_WIDTH) value = 128;

	for (uint16_t i = 0; i < value; ++i)
		if ((VRAM[i + offset] != WHITE)&&(VRAM[i + offset] != YELLOW)) VRAM[i + offset] = RED;
	for (uint16_t i = value; i < ST7789_WIDTH; ++i)
		if ((VRAM[i + offset] != WHITE)&&(VRAM[i + offset] != YELLOW)) VRAM[i + offset] = BLACK;
}

/**********************************************************************
 * column - maximum 25; value - maximum 128
 */
void ST7789_DrawColumn_for_Audio(int16_t column, int16_t value)
{
	uint16_t offset;

	if (value > ST7789_WIDTH) value = 128;

	for (uint8_t j = 0; j < 5; ++j)
	{
		offset = column * 6 * ST7789_WIDTH + ST7789_WIDTH * j;

		for (uint16_t i = 0; i < value; ++i)
			if (VRAM[i + offset] != WHITE) VRAM[i + offset] = RED;

		for (uint16_t i = value; i < ST7789_WIDTH; ++i)
			if (VRAM[i + offset] != WHITE) VRAM[i + offset] = BLACK;
	}

	offset = column * 6 * ST7789_WIDTH + ST7789_WIDTH * 5;

	for (uint16_t i = 0; i < ST7789_WIDTH; ++i)
		if (VRAM[i + offset] != WHITE) VRAM[i + offset] = BLACK;


	if (column == 24)	//
	{
		for (uint8_t j = 0; j < 10; ++j)
		{
			offset = 25 * 6 * ST7789_WIDTH + ST7789_WIDTH * j;

			for (uint16_t i = 0; i < ST7789_WIDTH; ++i)
				if (VRAM[i + offset] != WHITE) VRAM[i + offset] = BLACK;
		}
	}
}
/**********************************************************************/
void ST7789_DrawGraphNet_Mic()
{
	ST7789_DrawFillRect(0, 0, 110, 160, BLACK, updateVRAM);
	ST7789_DrawLine(110, 25, 110, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(110, 25, 0, 25, YELLOW, updateVRAM);
	ST7789_DrawLine(90, 25, 90, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(70, 25, 70, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(50, 25, 50, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(30, 25, 30, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(10, 25, 10, 160, YELLOW, updateVRAM);
	ST7789_DrawText_5x8(18, 16, YELLOW, "0", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 34, YELLOW, "-20", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 54, YELLOW, "-40", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 74, YELLOW, "-60", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 94, YELLOW, "-80", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(0, 114, YELLOW, "-100", Right, updateVRAM,BLACK);
}

/**********************************************************************/
void ST7789_DrawGraphNet_ADC()
{
	ST7789_DrawFillRect(0, 0, 110, 160, BLACK, updateVRAM);
	ST7789_DrawLine(110, 25, 110, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(110, 25, 0, 25, YELLOW, updateVRAM);
	ST7789_DrawLine(83, 25, 83, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(55, 25, 55, 160, YELLOW, updateVRAM);
	ST7789_DrawLine(27, 25, 27, 160, YELLOW, updateVRAM);
	ST7789_DrawText_5x8(18, 16, YELLOW, "0", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 41, YELLOW, "-20", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 68, YELLOW, "-40", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 95, YELLOW, "-60", Right, updateVRAM,BLACK);
	ST7789_DrawText_5x8(6, 122, YELLOW, "-80", Right, updateVRAM,BLACK);
}
/**********************************************************************/
void ST7789_DrawGraphClean()
{
	ST7789_DrawFillRect(0, 0, 113, 160, BLACK, updateVRAM);
}
#endif /* LCD_ANALYZER_H_ */

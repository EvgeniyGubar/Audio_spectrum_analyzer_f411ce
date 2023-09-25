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
		if (VRAM[i + offset] != WHITE) VRAM[i + offset] = BLUE;
	for (uint16_t i = value; i < ST7789_WIDTH; ++i)
		if (VRAM[i + offset] != WHITE) VRAM[i + offset] = BLACK;
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
			if (VRAM[i + offset] != WHITE) VRAM[i + offset] = BLUE;

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

#endif /* LCD_ANALYZER_H_ */

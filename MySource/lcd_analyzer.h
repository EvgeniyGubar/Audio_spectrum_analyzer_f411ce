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
void ST7789_DrawLine_for_Analyzer(int16_t y, int16_t x_max)
{
	if (x_max > ST7789_WIDTH)
	{
		x_max = 128;
	}
//
	uint16_t offset = y * ST7789_WIDTH;
//	uint16_t offset1 = (y + 1) * ST7789_WIDTH * 2;
//	uint16_t offset2 = (y + 2) * ST7789_WIDTH * 2;
//	uint16_t offset3 = (y + 3) * ST7789_WIDTH * 2;

	for (uint16_t i = 0; i < x_max; ++i)
	{
		VRAM[i + offset] = BLUE;
//		VRAM[i + 1 + offset] = BLUE & 0xFF;
//		VRAM[i + offset1] = BLUE >> 8;
//		VRAM[i + 1 + offset1] = BLUE & 0xFF;
//		VRAM[i + offset2] = BLUE >> 8;
//		VRAM[i + 1 + offset2] = BLUE & 0xFF;
//		VRAM[i + offset3] = BLUE >> 8;
//		VRAM[i + 1 + offset3] = BLUE & 0xFF;
	}
	for (uint16_t i = x_max; i < ST7789_WIDTH; ++i)
	{
		VRAM[i + offset] = BLACK;
//		VRAM[i + 1 + offset] = BLACK & 0xFF;
//		VRAM[i + offset1] = BLACK >> 8;
//		VRAM[i + 1 + offset1] = BLACK & 0xFF;
//		VRAM[i + offset2] = BLACK >> 8;
//		VRAM[i + 1 + offset2] = BLACK & 0xFF;
//		VRAM[i + offset3] = BLACK >> 8;
//		VRAM[i + 1 + offset3] = BLACK & 0xFF;
	}
}

#endif /* LCD_ANALYZER_H_ */

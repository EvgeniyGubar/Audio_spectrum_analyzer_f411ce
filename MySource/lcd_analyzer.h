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
	uint16_t offset = y * ST7789_WIDTH;

	if (x_max > ST7789_WIDTH) x_max = 128;

	for (uint16_t i = 0; i < x_max; ++i)
		VRAM[i + offset] = BLUE;
	for (uint16_t i = x_max; i < ST7789_WIDTH; ++i)
		VRAM[i + offset] = BLACK;
}

#endif /* LCD_ANALYZER_H_ */

/*
 * ws2812.c
 *
 *  Created on: Jul 5, 2022
 *      Author: Evgeniy
 */

#include "ws2812.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim2;
//extern DMA_HandleTypeDef hdma_tim2_ch2_ch4;

uint8_t led_PWM_data[LED_BUFF_LEN];

/************************************************************************/
/*    Функция конвертации HSV в RGB и последующее заполнение буфера     */
/************************************************************************/
// Converts a color from HSV to RGB.
// h is hue, as a number between 0 and 360.
// s is the saturation, as a number between 0 and 255.
// v is the value, as a number between 0 and 255.
void ws2812_hsv_to_buf(uint8_t h, uint8_t s, uint8_t v, uint16_t pos)
{
	uint8_t red, green, blue;

	if(s == 0)	red = green = blue = v;
	else
	{
		uint8_t region = h / 43;
		uint8_t remainder = (h - (region * 43)) * 6;
		uint8_t p = (v * (255 - s)) >> 8;
		uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
		uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

		switch(region)
		{
			case 0: red = v;	green = t;		blue = p;	break;
			case 1: red = q; 	green = v;		blue = p; 	break;
			case 2: red = p; 	green = v; 		blue = t; 	break;
			case 3: red = p;	green = q; 		blue = v;	break;
			case 4: red = t; 	green = p;		blue = v;	break;
			case 5: red = v; 	green = p;		blue = q;	break;
		}
	}
	ws2812_rgb_to_buf(red, green, blue, pos);
}
/************************************************************************/
/*        Заполнение буфера RGB форматом по указанной позиции           */
/************************************************************************/
void ws2812_rgb_to_buf(uint8_t red , uint8_t green, uint8_t blue, uint16_t pos)
	{
	  for(uint8_t i = 0; i < 8; ++i)
	  {
	    if (BIT_IS_SET(red,(7-i)) == 1)		led_PWM_data[DELAY_LEN+pos*24+8+i] = HIGH;
	    	else							led_PWM_data[DELAY_LEN+pos*24+8+i] = LOW;

	    if (BIT_IS_SET(green,(7-i)) == 1)	led_PWM_data[DELAY_LEN+pos*24+i] = HIGH;
	    	else							led_PWM_data[DELAY_LEN+pos*24+i] = LOW;

	    if (BIT_IS_SET(blue,(7-i)) == 1)	led_PWM_data[DELAY_LEN+pos*24+16+i] = HIGH;
	    	else							led_PWM_data[DELAY_LEN+pos*24+16+i] = LOW;
	   }
	}

/************************************************************************/
/*        														        */
/************************************************************************/
void ws2812_init(void)
{
	for (uint16_t i = 0; i < LED_BUFF_LEN; ++i)
		led_PWM_data[i] = (i < DELAY_LEN) ? 0 : 30;
	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)led_PWM_data, LED_BUFF_LEN);
}
/************************************************************************/
/*        														        */
/************************************************************************/
void ws2812_start(void)
{
	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)led_PWM_data, LED_BUFF_LEN);
}
/************************************************************************/
/*        														        */
/************************************************************************/
void ws2812_stop(void)
{
	HAL_TIM_PWM_Stop_DMA(&htim2,TIM_CHANNEL_2);
}


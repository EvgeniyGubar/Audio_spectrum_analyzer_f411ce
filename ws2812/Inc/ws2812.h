/*
 * ws2812.h
 *
 *  Created on: Jul 5, 2022
 *      Author: Evgeniy
 */

#ifndef LIBS_WS2812_H_
#define LIBS_WS2812_H_

#include "stdio.h"

#define HIGH 58
#define LOW 30
#define BIT_IS_SET(reg, bit)	((reg & (1<<bit)) != 0)
//#define ArrSize			5	//для отладки ADC
#define NUM_OF_LED  	500	//количество диодов
#define DELAY_LEN		40	//низкий уровень при старте в 50мкс
#define LED_BUFF_LEN	NUM_OF_LED*24 + DELAY_LEN //размер буфера для хранения цвета

void ws2812_hsv_to_buf(uint8_t h, uint8_t s, uint8_t v, uint16_t pos);
void ws2812_rgb_to_buf(uint8_t red , uint8_t green, uint8_t blue, uint16_t pos);
void ws2812_init(void);
void ws2812_start(void);
void ws2812_stop(void);

#endif /* LIBS_WS2812_H_ */

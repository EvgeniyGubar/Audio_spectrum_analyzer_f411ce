/*
 * hundler.h
 *
 *  Created on: Oct 23, 2022
 *      Author: Evgeniy
 */

#ifndef INC_HUNDLER_H_
#define INC_HUNDLER_H_

#include "stm32f4xx_hal.h"

#define FHT_LEN		1024

uint8_t flag_data_processing;
int16_t generate_fx[FHT_LEN];

void fft_init();
void timer_for_triggering_adc_init();
void data_processing();

#endif /* INC_HUNDLER_H_ */

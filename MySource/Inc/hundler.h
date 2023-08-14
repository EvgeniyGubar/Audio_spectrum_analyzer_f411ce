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

uint8_t fft_data_ready;

void fft_init();
void createRTOS();
void fft_hardware_init();
void processingTask();

#endif /* INC_HUNDLER_H_ */

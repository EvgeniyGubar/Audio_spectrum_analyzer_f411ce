/*
 * windowing.h
 *
 *  Created on: May 7, 2023
 *      Author: Evgeniy
 */

#ifndef INC_WINDOWING_H_
#define INC_WINDOWING_H_

#include "stdio.h"

void applyHammingWindow(float *fx);

void generateSample(int16_t *fx, int16_t ifreq, int16_t iamplitude);

//float32_t getWindow(int i, int n, uint8_t window);

#endif /* INC_WINDOWING_H_ */

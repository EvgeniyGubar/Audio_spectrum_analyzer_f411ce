/*
 * hundler.h
 *
 *  Created on: Oct 23, 2022
 *      Author: Evgeniy
 */

#ifndef INC_ANALYZER_H_
#define INC_ANALYZER_H_

#include "stm32f4xx_hal.h"

#define FHT_LEN		1024

void menuInit();
void fftInit();
void RTOScreate();
void hardwareInit();

#endif /* INC_ANALYZER_H_ */

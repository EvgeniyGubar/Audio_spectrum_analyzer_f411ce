/************************************************************************
	sampling.c

    16-Bit Fast Hartley Transform - Linux test
	Generate a test sample using a sine-wave
    Copyright (C) 2013 Simon Inns

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Email: simon.inns@gmail.com (please use the forum for questions)
	Forum: http://www.waitingforfriday.com/forum

************************************************************************/

// Include configuration
#include "fhtConfig.h"

// Global includes
#include <stdio.h>
#include <math.h>

// Local includes
#include "sampling.h"

#define PI_DOUBLE 3.1415926535897931

// Generate a test sample
void generateSample(int16_t *fx, int16_t ifreq, int16_t iamplitude)
{
	 // Generate input sine-wave:
	double amplitude = (double)iamplitude; // Maximum = 16383
	double samplesPerSecond = 20000;
	double frequency = (double) ifreq;
	double radiansPerSecond = (2 * PI_DOUBLE) * frequency;
	double time = 0;

	int16_t i;
	int16_t pointer = 0;

	// Generate the required number of samples
	for (i = 0; i < FHT_LEN; i++)
	{
		double floatCalc = amplitude * sin(radiansPerSecond * time);
		fx[i] = (int16_t)round(floatCalc);
		time += 1 / samplesPerSecond;
		pointer++;
	}

	int16_t k;

	printf("Output from generateSample():\r\n");
	for (k = 0; k < FHT_LEN; k++)
	{
		if (k < FHT_LEN && k !=0) printf(", ");
		if (k % 16 == 0) printf("\r\n");
		printf("%6d", fx[k]);
	}
	printf("\r\n\r\n");
}

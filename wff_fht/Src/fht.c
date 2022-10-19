/************************************************************************
	fht.c

    16-Bit Fast Hartley Transform
	Radix-2 Decimation in Time FHT routine
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

// Local includes
#include "fht.h"

// Sine and Cosine scaling factor (1 << SC_SCALE)
// Note: must be a power of 2 - i.e. 10 = (1 << 10) = x1024
// This determines the scaling of the sine and cosine values
// used in the FHT.  The higher the value the better the
// accuracy but the bigger the chance of overflowing the
// 16-bit integer maximums (a scale of 10 is approximately
// x1000 or 3 decimal places of accuracy).
//
// SC_SCALE is set during table auto-generation and should
// not be changed here.

// Sine and Cosine tables for 32 to 256 elements (stored in RAM):
#if (FHT_LEN == 32)
#define SC_SCALE  10
// FHT sine and cosine tables for FHT with 32 elements:
const int16_t cosTable[5] = {-1024,     0,   724,   946,  1004};
const int16_t sinTable[5] = {    0,  1024,   724,   391,   199};
#elif (FHT_LEN == 64)
#define SC_SCALE  10
// FHT sine and cosine tables for FHT with 64 elements:
const int16_t cosTable[6] = {-1024,     0,   724,   946,  1004,  1019};
const int16_t sinTable[6] = {    0,  1024,   724,   391,   199,   100};
#elif (FHT_LEN == 128)
#define SC_SCALE  10
// FHT sine and cosine tables for FHT with 128 elements:
const int16_t cosTable[7] = {-1024,     0,   724,   946,  1004,  1019,  1022};
const int16_t sinTable[7] = {    0,  1024,   724,   391,   199,   100,    50};
#elif (FHT_LEN == 256)
#define SC_SCALE  10
// FHT sine and cosine tables for FHT with 256 elements:
const int16_t cosTable[8] = {-1024,     0,   724,   946,  1004,  1019,  1022,  1023};
const int16_t sinTable[8] = {    0,  1024,   724,   391,   199,   100,    50,    25};
#endif

// Fast Hartley Transform - Radix-2 Decimation In Time
// Using 'in place' 16 bit signed integer data where the
// input array doubles as the output array to reduce RAM
// foot-print.
//
// The code in this function is based on "An Algorithm
// for the Fast Hartley Transform" by Ronald F.Ullmann
// http://sepwww.stanford.edu/theses/sep38/38_29_abs
// and adapted from the Ratfor example code.
//
// This function takes a set of real numbers (fx[]) from
// 0 to FHT_LEN-1 and outputs a set of complex numbers
// with real parts in 0 to (FHT_LEN/2)-1 and imaginary
// parts from FHT_LEN/2 to FHT_LEN-1.
//
// The maximum range of input values to the FHT is
// -16383 to +16383.  In order to keep the transient
// calculations in the 16-bit range (-32767/+32768)
// the FHT function performs continuous scaling of
// the calculations.  Due to the scaling the result
// is always half of the input range (-8192 to +8191).
//
void fhtDitInt(int16_t *fx)
{
	int32_t dcos, dsin, fcos, fsin;
	int16_t istep, k;
	uint16_t n;
	int16_t temp16bit1, butdis, butloc;
	int32_t temp32bit1, temp32bit2;
	uint8_t tablePointer;

	// Permute - bit reversal
	butdis = 0;
	for (k = 0; k < FHT_LEN; k++)
	{
		if (k <= butdis)
		{
			temp16bit1 = fx[butdis];
			fx[butdis] = fx[k];
			fx[k] = temp16bit1;
		}
		butloc = FHT_LEN >> 1;

		while(butdis >= butloc && butloc > 0)
		{
			butdis -= butloc;
			butloc = butloc >> 1;
		}
		butdis += butloc;
	}
	// End permute
	#ifdef PRINTF_DEBUG
	#ifdef FHT_DEBUG
	printf("fhtDitInt(): Permute - bit reversal output:\r\n");
	outputfx(fx, FHT_LEN);
	#endif
	#endif

	// Start of FHT function
	n = 1;
	tablePointer = 0;

	while (n < FHT_LEN)
	{
		#ifdef PRINTF_DEBUG
		#ifdef FHT_DEBUG
		printf("fhtDitInt(): FHT Loop n = %d:\r\n", n);
		outputfx(fx, FHT_LEN);
		#endif
		#endif
		
		istep = n << 1;

		// Get the sine and cosine data from the look-up tables
		dcos = (int32_t)cosTable[tablePointer];
		dsin = (int32_t)sinTable[tablePointer];
		fcos = dcos;
		fsin = dsin;
		tablePointer++;

		// Zero Frequency loop
		for (k = 0; k < FHT_LEN; k += istep)
		{
			temp16bit1 = fx[k + n];
			fx[k + n] = (fx[k] - temp16bit1) >> 1;
			fx[k] = (fx[k] + temp16bit1) >> 1;
		}
		#ifdef PRINTF_DEBUG
		#ifdef FHT_DEBUG
		printf("fhtDitInt(): Zero Frequency loop output:\r\n");
		outputfx(fx, FHT_LEN);
		#endif
		#endif
		// End zero frequency loop

		if (n > 2)
		{
			// Double butterfly
			butdis = n - 2;
			for (butloc = 1; butloc < (n >> 1); butloc++)
			{
				if (n > 2)
				{
					// Double butterfly
					butdis = n - 2;
					for (butloc = 1; butloc < (n >> 1); butloc++)
					{
						for (k = butloc; k <= FHT_LEN; k += istep)
						{
							// Using 32 bit integers for the main multiplication to prevent overflow errors
							// and we add 1 to SC_SCALE to divide the result by 2 or the addition below can overflow
							temp32bit1 = ((fcos * (int32_t)fx[k + n]) + (fsin * (int32_t)fx[k + n + butdis])) >> (SC_SCALE + 1);
							temp32bit2 = ((fsin * (int32_t)fx[k + n]) - (fcos * (int32_t)fx[k + n + butdis])) >> (SC_SCALE + 1);

							fx[k + n] = (fx[k] >> 1) - (int16_t)temp32bit1;
							fx[k + n + butdis] = (fx[k + butdis] >> 1) - (int16_t)temp32bit2;
							fx[k] = (fx[k] >> 1) + (int16_t)temp32bit1;
							fx[k + butdis] = (fx[k + butdis] >> 1) + (int16_t)temp32bit2;
						}

						temp32bit1 = (fcos * dcos - fsin * dsin) >> SC_SCALE;
						fsin = (fsin * dcos + fcos * dsin) >> SC_SCALE;

						fcos = temp32bit1;
						butdis -= 2;
					}
					#ifdef PRINTF_DEBUG
					#ifdef FHT_DEBUG
					printf("fhtDitInt(): Double-butterfly output:\r\n");
					outputfx(fx, FHT_LEN);
					#endif
					#endif
					// End double butterfly
				}

				temp32bit1 = (fcos * dcos - fsin * dsin) >> SC_SCALE;
				fsin = (fsin * dcos + fcos * dsin) >> SC_SCALE;

				fcos = temp32bit1;
				butdis -= 2;
			}
			#ifdef PRINTF_DEBUG
			#ifdef FHT_DEBUG
			printf("fhtDitInt(): Double-butterfly output:\r\n");
			outputfx(fx, FHT_LEN);
			#endif
			#endif
			// End double butterfly
		}

		if (n > 1)
		{
			// Nyquist Frequency Loop
			for (k = (n >> 1); k < FHT_LEN; k += istep)
			{
				temp16bit1 = fx[k + n];
				fx[k + n] = (fx[k] - temp16bit1) >> 1;
				fx[k] = (fx[k] + temp16bit1) >> 1;
			}
			#ifdef PRINTF_DEBUG
			#ifdef FHT_DEBUG
			printf("fhtDitInt(): Nyquist Frequency loop output:\r\n");
			outputfx(fx, FHT_LEN);
			#endif
			#endif
			// End Nyquist frequency loop
		}
		n = istep;
	}
	// End FHT

	#ifdef PRINTF_DEBUG
	printf("Output from fhtDitInt():\r\n");
	outputfx(fx, FHT_LEN);
	#endif
}


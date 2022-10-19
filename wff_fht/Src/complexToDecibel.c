/************************************************************************
	complextodecibel.c

    16-Bit Fast Hartley Transform
	Functions to convert linear complex results into real dB results
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
#include "complexToDecibel.h"

// Linear to dB output tables for 64 to 128 scaled output (stored in PROGMEM):
#if (N_DB == 64)
// Linear complex number to real number dB table for output range of 63-0:
// Range -78 dB to 0 dB
const int16_t dbMap[64] PROGMEM = {
	1,     1,     1,     1,     2,     2,     2,     3,
	3,     4,     4,     5,     6,     7,     8,     9,
	11,    12,    14,    17,    19,    22,    25,    29,
	34,    39,    45,    52,    60,    69,    79,    91,
	105,   121,   140,   161,   185,   213,   245,   282,
	324,   373,   430,   494,   569,   655,   754,   867,
	998,  1148,  1321,  1520,  1749,  2013,  2316,  2665,
	3067,  3529,  4061,  4672,  5376,  6186,  7118,  8191
};
#elif (N_DB == 128)
// Linear complex number to real number dB table for output range of 127-0:
// Range -78 dB to 0 dB
const int16_t dbMap[128] PROGMEM = {
	1,     1,     1,     1,     1,     1,     1,     1,
	1,     2,     2,     2,     2,     2,     2,     3,
	3,     3,     3,     4,     4,     4,     5,     5,
	5,     6,     6,     7,     7,     8,     9,     9,
	10,    11,    12,    12,    13,    14,    15,    17,
	18,    19,    21,    22,    24,    25,    27,    29,
	32,    34,    36,    39,    42,    45,    48,    52,
	56,    60,    64,    69,    74,    79,    85,    91,
	98,   105,   113,   121,   130,   140,   150,   161,
	172,   185,   198,   213,   228,   245,   263,   282,
	302,   324,   348,   373,   401,   430,   461,   494,
	530,   569,   610,   655,   702,   754,   808,   867,
	930,   998,  1070,  1148,  1232,  1321,  1417,  1520,
	1631,  1749,  1877,  2013,  2159,  2316,  2485,  2665,
	2859,  3067,  3290,  3529,  3786,  4061,  4356,  4672,
	5012,  5376,  5767,  6186,  6636,  7118,  7636,  8191
};
#endif

// Linear to dB output tables with gain for 64 to 128 scaled output (stored in PROGMEM):
#if (N_DB == 64)
// Linear complex number to real number dB table for output range of 63-0:
// Range -78 dB to -30 dB
const int16_t dbMapPlusGain[64]  = {
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	1,     1,     1,     1,     1,     2,     2,     2,
	3,     3,     4,     5,     5,     6,     7,     8,
	10,    11,    13,    15,    18,    20,    23,    27,
	31,    36,    41,    48,    55,    63,    73,    84,
	97,   111,   128,   147,   170,   195,   225,   259
};
#elif (N_DB == 128)
// Linear complex number to real number dB table for output range of 127-0:
// Range -78 dB to -30 dB
const int16_t dbMapPlusGain[128] PROGMEM = {
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	1,     1,     1,     1,     1,     1,     1,     1,
	1,     1,     2,     2,     2,     2,     2,     2,
	3,     3,     3,     3,     4,     4,     4,     5,
	5,     5,     6,     6,     7,     7,     8,     8,
	9,    10,    11,    11,    12,    13,    14,    15,
	16,    18,    19,    20,    22,    23,    25,    27,
	29,    31,    33,    36,    38,    41,    44,    48,
	51,    55,    59,    63,    68,    73,    78,    84,
	90,    97,   104,   111,   119,   128,   137,   147,
	158,   170,   182,   195,   209,   225,   241,   259
};
#endif

// This function converts the complex number output from the FHT
// into real number logarithmic decibel values.
//
// The stages are:
// 1. convert to magnitude values: magnitude = sqrt(re * re + im * im)
// 2. convert to dB (log) scale dB = 20 * log10(magnitude)
//
// However we can combine these two steps to save processing
// since db = 10 * log10(re * re + im * im) is mathematically
// equivalent to the previous two steps and saves us from having
// to perform the sqrt().
//
void complexToDecibel(int16_t *fx)
{
	// Process for each complex number in fx[FHT_LEN/2]
	int16_t k, i;
	int32_t calc;

	for (k = 0; k < FHT_LEN / 2; k++)
	{
		// calc = (fx(k)^2) + (fx(-k)^2)
		calc = ((int32_t)fx[k] * (int32_t)fx[k] +
		(int32_t)fx[FHT_LEN - k] * (int32_t)fx[FHT_LEN - k]);

		// The table expects a maximum result of 16383
		// Since the maximum output from the FHT is as follows:
		// = 16383 * 16383 + -16383 * -16383
		// The maximum result of the above calculation is:
		// = 536805378
		//
		// 536805378 / 16834 = 16384 = 1 << 14
		//
		// Therefore we scale like this:
		calc = calc >> 14;

		// Now we seek the position in the decibel table which contains
		// a lower value than calc.  The position in the table gives
		// us the approximate dB level
		for (i = 0; i < N_DB; i++)
		{
#ifdef AVR_GCC
			// Read table from program memory
			if (calc <= pgm_read_word(&dbMap[i])) break;
#else
			// Read table from RAM
			if (calc <= dbMap[i]) break;
#endif
		}

		// Check for overflow in the case that we didn't break
		// from the for loop
		if (i == N_DB) i = N_DB - 1;

		// Now we write the decibel value back into fx[k]
		fx[k] = i;
	}

#ifdef PRINTF_DEBUG
	printf("Output from complexToDecibel():\r\n");
	outputfx(fx, FHT_LEN/2);
#endif
}

// This function is identical to the previous function
// but uses a table with built in gain for a higher 
// output signal
void complexToDecibelWithGain(int16_t *fx)
{
	// Process for each complex number in fx[FHT_LEN/2]
	int16_t k, i;
	int32_t calc;

	for (k = 0; k < FHT_LEN / 2; k++)
	{
		// calc = (fx(k)^2) + (fx(-k)^2)
		calc = ((int32_t)fx[k] * (int32_t)fx[k] +
		(int32_t)fx[FHT_LEN - k] * (int32_t)fx[FHT_LEN - k]);

		// The table expects a maximum result of 16383
		// Since the maximum output from the FHT is as follows:
		// = 16383 * 16383 + -16383 * -16383
		// The maximum result of the above calculation is:
		// = 536805378
		//
		// 536805378 / 16834 = 16384 = 1 << 14
		//
		// Therefore we scale like this:
		calc = calc >> 14;

		// Now we seek the position in the decibel table which contains
		// a lower value than calc.  The position in the table gives
		// us the approximate dB level
		for (i = 0; i < N_DB; i++)
		{
			#ifdef AVR_GCC
			// Read table from program memory
			if (calc <= pgm_read_word(&dbMapPlusGain[i])) break;
			#else
			// Read table from RAM
			if (calc <= dbMapPlusGain[i]) break;
			#endif
		}

		// Check for overflow in the case that we didn't break
		// from the for loop
		if (i == N_DB) i = N_DB - 1;

		// Now we write the decibel value back into fx[k]
		fx[k] = i;
	}

	#ifdef PRINTF_DEBUG
	printf("Output from complexToDecibelWithGain():\r\n");
	outputfx(fx, FHT_LEN/2);
	#endif
}

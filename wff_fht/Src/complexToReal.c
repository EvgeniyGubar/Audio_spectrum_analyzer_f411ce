/************************************************************************
	complextoreal.c

    16-Bit Fast Hartley Transform
	Functions to convert complex output to linear real numbers
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
#include "complexToReal.h"

#include <stdlib.h>

// This function calculates the absolute value of the
// complex numbers returned by the FHT and outputs real
// number results.
//
// This calculates the linear amplitude (magnitude)
// spectrum of the FHT results using the following formula:
// fx(k) = SQRT( (fx(k)^2) + (fx(-k)^2) )
//
// The function calculates the square-roots using a
// fast (but compiler/processor dependent) integer
// approximation adapted from code at:
// http://www.codecodex.com/wiki/Calculate_an_integer_square_root
//
// Finding the SQRT of a 32-bit number using look-up tables
// requires a very big table - so this is probably the better
// option for low RAM footprint unless you want to reduce the
// output resolution
//
// Note: The output from this function is 0-8191 if scale = 0
// Setting scale to non-zero causes the output to be scaled down
// by output >> scale, i.e. a scale of 8 will output 0-63.
void complexToReal(int16_t *fx, int16_t scale)
{
	int32_t place, root, calc;
	int16_t k;

	for (k = 0; k < (FHT_LEN / 2); k++)
	{
		calc = ((int32_t)fx[k] * (int32_t)fx[k] +
		(int32_t)fx[FHT_LEN - k] * (int32_t)fx[FHT_LEN - k]);

		// Find the square root of the 32bit number
		place = 0x40000000;
		root = 0;

		// Ensure we don't have a negative number
		if (calc >= 0)
		{
			while (place > calc) place = place >> 2;

			while (place)
			{
				if (calc >= root + place)
				{
					calc -= root + place;
					root += place << 1;
				}
				root = root >> 1;
				place = place >> 2;
			}
		}
		// Now scale back up to 16 bits

		// The FHT's maximum input range is -16383 to +16383
		// and the FHT output is -16383 to +16383 (for both
		// the real and imaginary number parts)
		//
		// Therefore the maximum real linear amplitude of a
		// frequency bin is:
		// = 8191 * 8191 + -8192 * -8192
		// = 134201345
		// = SQRT(134201345)
		// = 11585
		//
		// To avoid a floating-point multiplication by *(16383/23169) =
		// 0.707108637 we can approximate the percentage by using a fraction
		// which has a power-of-two denominator (in other words we
		// scale up the result (since we have a 32 bit variable) and
		// then divide it by 2^15.
		//
		// root = root * 11584 / 16384 = root * 11584 >> 14
		// and we also add in the additional scaling division:
		root = root * 11584 >> (14 + scale); // Outputs 0-8191 >> scale from 0-11584
		
		// Scale the result and store it
		fx[k] = (int16_t)root;
	}

	#ifdef PRINTF_DEBUG
	printf("Output from complexToReal():\r\n");
	outputfx(fx, FHT_LEN/2);
	#endif
}








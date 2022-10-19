/************************************************************************
	fhtDebug.c

    16-Bit Fast Hartley Transform
	Debug output functions
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
#include "fhtDebug.h"

#ifdef PRINTF_DEBUG
// Outputs a formated table showing the contents of fx[]
// to the printf stream
void outputfx(int16_t *fx, int16_t len)
{
	// Debug output counter
	int16_t k;

	for (k = 0; k < len; k++)
	{
		if (k < len && k !=0) printf(", ");
		if (k % 16 == 0) printf("\r\n");
		printf("%6d", fx[k]);
	}
	printf("\r\n\r\n");
}
#endif

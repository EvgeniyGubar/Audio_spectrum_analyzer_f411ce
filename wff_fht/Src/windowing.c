/************************************************************************
	windowing.c

    16-Bit Fast Hartley Transform
	Sample windowing functions
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
#include "windowing.h"

// Hamming window tables for 32 to 256 elements (stored in PROGMEM):
#if (FHT_LEN == 32)
// Hamming window table for samples with 32 elements (scaled x16384):
// Since the hamming window is symmetrical we only need half of the window
const int16_t hammingTable[16] PROGMEM = {
	1310,  1464,  1921,  2661,  3654,  4860,  6229,  7706,  9229, 10736, 12166, 13460, 14565, 15436, 16038, 16345
};
#elif (FHT_LEN == 64)
// Hamming window table for samples with 64 elements (scaled x16384):
// Since the hamming window is symmetrical we only need half of the window
const int16_t hammingTable[32] PROGMEM = {
	1310,  1348,  1460,  1645,  1902,  2228,  2620,  3073,  3585,  4148,  4758,  5409,  6093,  6806,  7538,  8284,
	9035,  9784, 10524, 11247, 11947, 12615, 13246, 13834, 14372, 14855, 15278, 15637, 15929, 16150, 16299, 16374
};
#elif (FHT_LEN == 128)
// Hamming window table for samples with 128 elements (scaled x16384):
// Since the hamming window is symmetrical we only need half of the window
const int16_t hammingTable[64] PROGMEM = {
	1310,  1319,  1347,  1393,  1457,  1540,  1640,  1758,  1893,  2045,  2214,  2399,  2600,  2816,  3047,  3292,
	3551,  3822,  4106,  4402,  4708,  5025,  5351,  5685,  6027,  6377,  6732,  7092,  7457,  7825,  8195,  8567,
	8940,  9313,  9684, 10053, 10420, 10782, 11140, 11493, 11838, 12177, 12507, 12828, 13140, 13441, 13731, 14009,
	14274, 14526, 14764, 14988, 15196, 15389, 15566, 15727, 15871, 15997, 16106, 16197, 16271, 16326, 16363, 16381
};
#elif (FHT_LEN == 256)
// Hamming window table for samples with 256 elements (scaled x16384):
// Since the hamming window is symmetrical we only need half of the window
const int16_t hammingTable[128] PROGMEM = {
	1310,  1313,  1319,  1331,  1347,  1367,  1392,  1422,  1456,  1495,  1538,  1585,  1637,  1694,  1754,  1819,
	1888,  1962,  2039,  2121,  2207,  2297,  2391,  2488,  2590,  2695,  2805,  2917,  3034,  3154,  3277,  3404,
	3534,  3667,  3804,  3943,  4086,  4231,  4379,  4530,  4684,  4840,  4998,  5159,  5322,  5487,  5655,  5824,
	5995,  6168,  6342,  6518,  6695,  6874,  7054,  7235,  7416,  7599,  7783,  7967,  8151,  8337,  8522,  8708,
	8893,  9079,  9264,  9450,  9635,  9819, 10003, 10186, 10368, 10550, 10730, 10909, 11087, 11264, 11439, 11613,
	11785, 11955, 12123, 12289, 12453, 12615, 12775, 12932, 13087, 13239, 13389, 13536, 13679, 13820, 13958, 14093,
	14225, 14354, 14479, 14600, 14719, 14833, 14944, 15051, 15155, 15255, 15350, 15442, 15530, 15614, 15694, 15769,
	15840, 15908, 15970, 16029, 16083, 16133, 16178, 16219, 16255, 16287, 16314, 16337, 16355, 16369, 16378, 16383
};
#endif

// Hann window tables for 32 to 256 elements (stored in PROGMEM):
#if (FHT_LEN == 32)
// Hann window table for samples with 32 elements (scaled x16384):
// Since the hann window is symmetrical we only need half of the window
const int16_t hannTable[16] PROGMEM = {
	0,   167,   663,  1468,  2547,  3858,  5346,  6951,  8606, 10245, 11799, 13206, 14407, 15354, 16008, 16341
};
#elif (FHT_LEN == 64)
// Hann window table for samples with 64 elements (scaled x16384):
// Since the hann window is symmetrical we only need half of the window
const int16_t hannTable[32] PROGMEM = {
	0,    40,   162,   363,   643,   997,  1423,  1916,  2472,  3084,  3747,  4454,  5199,  5973,  6769,  7579,
	8396,  9210, 10014, 10801, 11561, 12288, 12974, 13612, 14197, 14722, 15182, 15572, 15889, 16130, 16292, 16373
};
#elif (FHT_LEN == 128)
// Hann window table for samples with 128 elements (scaled x16384):
// Since the hann window is symmetrical we only need half of the window
const int16_t hannTable[64] PROGMEM = {
	0,    10,    40,    90,   159,   249,   358,   486,   633,   798,   982,  1183,  1401,  1636,  1887,  2154,
	2435,  2730,  3039,  3360,  3693,  4037,  4391,  4755,  5127,  5506,  5892,  6284,  6680,  7080,  7483,  7888,
	8293,  8698,  9102,  9503,  9901, 10295, 10684, 11067, 11443, 11811, 12170, 12519, 12858, 13185, 13500, 13802,
	14091, 14364, 14623, 14866, 15093, 15303, 15495, 15670, 15826, 15964, 16082, 16181, 16261, 16321, 16361, 16381
};
#elif (FHT_LEN == 256)
// Hann window table for samples with 256 elements (scaled x16384):
// Since the hann window is symmetrical we only need half of the window
const int16_t hannTable[128] PROGMEM = {
	0,     2,     9,    22,    39,    62,    89,   121,   158,   200,   247,   299,   355,   416,   482,   553,
	628,   708,   792,   881,   974,  1072,  1174,  1280,  1391,  1505,  1624,  1746,  1873,  2003,  2138,  2275,
	2417,  2562,  2710,  2862,  3017,  3175,  3336,  3500,  3667,  3836,  4008,  4183,  4360,  4540,  4722,  4906,
	5092,  5279,  5469,  5660,  5853,  6047,  6242,  6439,  6637,  6835,  7035,  7235,  7436,  7637,  7838,  8040,
	8242,  8444,  8645,  8847,  9048,  9248,  9448,  9647,  9845, 10042, 10239, 10433, 10627, 10819, 11009, 11198,
	11385, 11570, 11752, 11933, 12111, 12288, 12461, 12632, 12800, 12966, 13128, 13288, 13444, 13598, 13748, 13894,
	14037, 14177, 14313, 14445, 14574, 14698, 14819, 14936, 15048, 15156, 15261, 15360, 15456, 15547, 15634, 15716,
	15793, 15866, 15934, 15998, 16057, 16111, 16160, 16204, 16244, 16279, 16308, 16333, 16353, 16368, 16378, 16383
};
#endif

// Apply a Hamming window to the input sample data
void applyHammingWindow(int16_t *fx)
{
	int16_t k, i = 0;
	int32_t calc;

	// First half of the window
	for (k = 0; k < FHT_LEN/2; k++)
	{
#ifdef AVR_GCC
		// Read table from program memory
		calc = ((int32_t)fx[i] * (int32_t)pgm_read_word(&hammingTable[k])) >> 14;
		fx[i] = (int16_t)calc;
#else
		// Read table from RAM
		calc = ((int32_t)fx[i] * (int32_t)hammingTable[k]) >> 14;
		fx[i] = (int16_t)calc;
#endif
		i++;
	}

	// Second half of the window
	for (k = (FHT_LEN/2)-1; k >= 0; k--)
	{
#ifdef AVR_GCC
		// Read table from program memory
		calc = ((int32_t)fx[i] * (int32_t)pgm_read_word(&hammingTable[k])) >> 14;
		fx[i] = (int16_t)calc;
#else
		// Read table from RAM
		calc = ((int32_t)fx[i] * (int32_t)hammingTable[k]) >> 14;
		fx[i] = (int16_t)calc;
#endif
		i++;
	}

#ifdef PRINTF_DEBUG
	printf("Output from applyHammingWindow():\r\n");
	outputfx(fx, FHT_LEN);
#endif
}

// Apply a Hann window to the input sample data
void applyHannWindow(int16_t *fx)
{
	int16_t k, i = 0;
	int32_t calc;

	// First half of the window
	for (k = 0; k < FHT_LEN/2; k++)
	{
#ifdef AVR_GCC
		// Read table from program memory
		calc = ((int32_t)fx[i] * (int32_t)pgm_read_word(&hannTable[k])) >> 14;
		fx[i] = (int16_t)calc;
#else
		// Read table from RAM
		calc = ((int32_t)fx[i] * (int32_t)hannTable[k]) >> 14;
		fx[i] = (int16_t)calc;
#endif
		i++;
	}

	// Second half of the window
	for (k = (FHT_LEN/2)-1; k >= 0; k--)
	{
#ifdef AVR_GCC
		// Read table from program memory
		calc = ((int32_t)fx[i] * (int32_t)pgm_read_word(&hannTable[k])) >> 14;
		fx[i] = (int16_t)calc;
#else
		// Read table from RAM
		calc = ((int32_t)fx[i] * (int32_t)hannTable[k]) >> 14;
		fx[i] = (int16_t)calc;
#endif
		i++;
	}

#ifdef PRINTF_DEBUG
	printf("Output from applyHannWindow():\r\n");
	outputfx(fx, FHT_LEN);
#endif
}

/************************************************************************
	generateTables.h

    16-Bit Fast Hartley Transform
	Generate the pre-calculated tables for the FHT library
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
#include "generateTables.h"

#ifdef GENERATE_TABLES

// Generate scaled sine and cosine tables for the fht_dit_int function
//
// The required length of the table is dependent on the length of the
// input data to the FHT and is the same length as FHT_SCALE; i.e. if
// the FHT_LEN is 128 and FHT_SCALE is 7 the required table length is
// 7 elements.
//
// As this table is generally very small it can be stored in RAM for speed
void generateFhtCosTable(int16_t fht_len, int16_t fht_scale, int16_t sc_len)
{
	double arg;
	int16_t tablePointer = 0, n = 1, istep;

	arg = 3.14159265359; 	// Pi

	printf("    // FHT sine and cosine tables for FHT with %d elements:\r\n", fht_len);
	printf("    const int16_t cosTable[%d] = {", fht_scale);
	while (n < fht_len)
	{
		if (n != 1) printf(", ");
		istep = 2 * n;

		// Calculate the cosine value and then scale the result:
		printf("%5d", (int16_t)(cos(arg) * sc_len));

		arg = arg / 2;
		n = istep;
		tablePointer++;
	}
	printf("};\r\n");
}

void generateFhtSinTable(int16_t fht_len, int16_t fht_scale, int16_t sc_len)
{
	double arg;
	int16_t tablePointer = 0, n = 1, istep;

	arg = 3.14159265359; 	// Pi
	printf("    const int16_t sinTable[%d] = {", fht_scale);
	while (n < fht_len)
	{
		if (n != 1) printf(", ");
		istep = 2 * n;

		// Calculate the sine value and then scale the result:
		printf("%5d", (int16_t)(sin(arg) * sc_len));

		arg = arg / 2;
		n = istep;
		tablePointer++;
	}
	printf("};\r\n");
}

// This function generates the Hamming window data and scales it by 16384
// Note: Only half of the function is generated since the window is symmetrical and
// we don't need the other half
void generateHammingWindowTable(int16_t fht_len)
{
	// Generate Hamming Window data
	// One datum for each sample
	int16_t k;
	double hammingCalc;
	printf("    // Hamming window table for samples with %d elements (scaled x16384):\r\n", fht_len);
	printf("    // Since the hamming window is symmetrical we only need half of the window\r\n");
	printf("    const int16_t hammingTable[%d] PROGMEM = {", fht_len/2);
	for (k = 0; k < fht_len/2; k++)
	{
		hammingCalc = 16384 * (0.54 - 0.46 * cos(2 * 3.14159265359 * (double)k / (fht_len - 1)));
		if (k < (fht_len) && k !=0) printf(", ");
		if (k % 16 == 0) printf("\r\n    ");
		printf("%5d",(int16_t)hammingCalc);
	}
	printf("\r\n    };\r\n");
}

// This function generates the Hann window data and scales it by 16384
// Note: Only half of the function is generated since the window is symmetrical and 
// we don't need the other half
void generateHannWindowTable(int16_t fht_len)
{
	// Generate Hann Window data
	// One datum for each sample
	int16_t k;
	double hannCalc;
	printf("    // Hann window table for samples with %d elements (scaled x16384):\r\n", fht_len);
	printf("    // Since the hann window is symmetrical we only need half of the window\r\n");
	printf("    const int16_t hannTable[%d] PROGMEM = {", fht_len/2);
	for (k = 0; k < fht_len/2; k++)
	{
		hannCalc = 16384 * (0.5 - 0.5 * cos(2 * 3.14159265359 * (double)k / (fht_len - 1)));
		if (k < (fht_len) && k !=0) printf(", ");
		if (k % 16 == 0) printf("\r\n    ");
		printf("%5d",(int16_t)hannCalc);
	}
	printf("\r\n    };\r\n");
}

// Generate dB map table
//
// To get the dB we have to have a reference value.  The reference
// point is the input V when dBV = 0 (i.e. maximum intensity).
//
// To get from voltage (V) to voltage level (dBV) we need the following
// formula:
//
// dBV = 20 * log10(V / refV)
//
// where refV is our dBV = 0 reference point.
//
// To create the table we need to do the reverse and translate the
// intensity into N_LOUD steps of dB from 0 to the noise floor.
//
// The reverse of the above equation is:
//
// V = refV * 10^(dBV / 20)
//
// To generate the table you must decide:
//
// steps = the number of elements in the array.  In this case we are using
// 64 steps which allows us to output the logarithmic representation on
// a gLCD without further scaling.
//
// Noise floor = the noise floor of the FHT is about -78dBs.
// (dB info from http://http://wiki.openmusiclabs.com/wiki/ArduinoFHT)
//
// zero dB reference = the maximum possible intensity (32767 in this case)
//
// Note: the 'real' dB is based on the variables mentioned above.  Effectively.
// if your scale is 75 dB to 0 dB then each element of the array represents
// -75 dB / N_LOUD - so ?1.172 dB per step (over 64 steps).
//
// Note: The table is reversed since we are more likely to have a
// low dB result than a high dB result.  This means we match faster
// against quieter results.
void generatedBMap(int16_t n_loud, int16_t iNoiseFloor)
{
	// Firstly we calculate the number of dBs we step per
	// element of the overall array (N_LOUD)
	double noiseFloor = (double)iNoiseFloor;
	double dbStep = noiseFloor / n_loud;

	// Set our zero dB reference value
	double zeroDbReference = 8191;

	// Define a double for storing the floating point
	// calculation
	double calc;

	// Double to store the current dB level
	double currentLevel = noiseFloor - dbStep;

	// An int16_t to store the result of the calculation
	int16_t result;

	// Define a pointer to the array
	int16_t k;

	// Define a counter for formatting the result
	int16_t i = 0;

	printf("    // Linear complex number to real number dB table for output range of %d-0:\r\n", n_loud - 1);
	printf("    // Range %d dB to 0 dB\r\n", (int16_t)noiseFloor);
	printf("    const int16_t dbMap[%d] PROGMEM = {", n_loud);
	for (k = n_loud - 1; k >= 0; k--)
	{
		// Calculate the result
		calc = zeroDbReference * pow(10, currentLevel / 20);
		result = (int16_t)calc;

		if (i < n_loud && i !=0) printf(", ");
		if (i % 8 == 0) printf("\r\n    ");
		printf("%5d", result);

		// Increment the current level
		currentLevel -= dbStep;

		i++;
	}
	printf("\r\n    };\r\n");
}

// The dB map with gain table is similar to the dbMap table
// however you can specify the overall gain and change
// the noise floor.
//
// This isn't so useful for spectrum analysis (except if you
// set it to a noise floor of -75 dB and a gain of 6 dB to
// counter the spectrum intensity loss due to windowing).
//
// You can use this to specify a much smaller 'range' to display.
// For example if you set the noise floor to -40 and the gain
// to -30 you effectively output data in the range -40 to -30 with
// all values over -30 dB clipped.  This produces a nice 'lively'
// spectra output which looks nice if you are only interested in
// a good bouncy display - and, hey, that's ok too ;)
//
// Note: The table is reversed since we are more likely to have a
// low dB result than a high dB result.  This means we match faster
// against quieter results.
void generatedBMapWithGain(int16_t n_loud, int16_t iNoiseFloor, int16_t gain)
{
	// Firstly we calculate the number of dBs we step per
	// element of the overall array (N_LOUD)
	double noiseFloor = (double)iNoiseFloor;
	double dbStep = noiseFloor / n_loud;

	// Set our zero dB reference value
	double zeroDbReference = 8191 * pow(10, ((double)gain / 20));

	// Define a double for storing the floating point
	// calculation
	double calc;

	// Double to store the current dB level
	double currentLevel = noiseFloor - dbStep;

	// An int16_t to store the result of the calculation
	int16_t result;

	// Define a pointer to the array
	int16_t k;

	// Define a counter for formatting the result
	int16_t i = 0;

	printf("    // Linear complex number to real number dB table for output range of %d-0:\r\n", n_loud - 1);
	printf("    // Range %d dB to %d dB\r\n", (int16_t)noiseFloor, gain);
	printf("    const int16_t dbMapPlusGain[%d] PROGMEM = {", n_loud);
	for (k = n_loud - 1; k >= 0; k--)
	{
		// Calculate the result
		calc = zeroDbReference * pow(10, currentLevel / 20);
		result = (int16_t)calc;

		if (i < n_loud && i !=0) printf(", ");
		if (i % 8 == 0) printf("\r\n    ");
		printf("%5d", result);

		// Increment the current level
		currentLevel -= dbStep;

		i++;
	}
	printf("\r\n    };\r\n");
}

void generateTables(void)
{
	// Minimum and maximum sizes to create tables for:
	int16_t minScale = 5; //  32 elements
	int16_t maxScale = 8; // 256 elements

	int16_t scale;

	// Generate tables for 32 to 256 elements
	printf("// Sine and Cosine tables for %d to %d elements (stored in RAM):\r\n", 1 << minScale, 1 << maxScale);
	for (scale = minScale; scale <= maxScale; scale++)
	{
		if (scale == minScale) printf("#if (FHT_LEN == %d)\r\n", 1 << scale);
		else printf("#elif (FHT_LEN == %d)\r\n", 1 << scale);

		printf("#define SC_SCALE  10\r\n"); // Sin/cos scaling x1024 (1 << 10)
		generateFhtCosTable(1 << scale, scale, 1024); // Sin/cos scaling x1024
		generateFhtSinTable(1 << scale, scale, 1024);
	}
	printf("#endif\r\n\r\n");

	printf("// Hamming window tables for %d to %d elements (stored in PROGMEM):\r\n", 1 << minScale, 1 << maxScale);
	for (scale = minScale; scale <= maxScale; scale++)
	{
		if (scale == minScale) printf("#if (FHT_LEN == %d)\r\n", 1 << scale);
		else printf("#elif (FHT_LEN == %d)\r\n", 1 << scale);

		generateHammingWindowTable(1 << scale);
	}
	printf("#endif\r\n\r\n");

	printf("// Hann window tables for %d to %d elements (stored in PROGMEM):\r\n", 1 << minScale, 1 << maxScale);
	for (scale = minScale; scale <= maxScale; scale++)
	{
		if (scale == minScale) printf("#if (FHT_LEN == %d)\r\n", 1 << scale);
		else printf("#elif (FHT_LEN == %d)\r\n", 1 << scale);

		generateHannWindowTable(1 << scale);
	}
	printf("#endif\r\n\r\n");

	// Generate the dbMap
	printf("// Linear to dB output tables for %d to %d scaled output (stored in PROGMEM):\r\n", 1 << 6, 1 << 7);
	for (scale = 6; scale <= 7; scale++)
	{
		if (scale == 6) printf("#if (N_DB == %d)\r\n", 1 << scale);
		else printf("#elif (N_DB == %d)\r\n", 1 << scale);

		// Parameters are scale and noise floor (in dB)
		generatedBMap(1 << scale, -78);
	}
	printf("#endif\r\n\r\n");

	// Generate the dbMap with gain
	printf("// Linear to dB output tables with gain for %d to %d scaled output (stored in PROGMEM):\r\n", 1 << 6, 1 << 7);
	for (scale = 6; scale <= 7; scale++)
	{
		if (scale == 6) printf("#if (N_DB == %d)\r\n", 1 << scale);
		else printf("#elif (N_DB == %d)\r\n", 1 << scale);

		// parameters are scale, noise floor (in dB) and  dB gain
		generatedBMapWithGain(1 << scale, -78, -30); // 12 dB gain
	}
	printf("#endif\r\n\r\n");
}

#endif


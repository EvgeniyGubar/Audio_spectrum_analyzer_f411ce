/************************************************************************
	fhtConfig.h

    16-Bit Fast Hartley Transform
	Configuration for the FHT and associated functions
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

#ifndef FHTCONFIG_H_
#define FHTCONFIG_H_

// The FHT length is the number of input datum to the FHT (i.e. the number of samples)
// this is defined by FHT_SCALE which denotes 2^6 = 64 where FHT_SCALE = 6
// You must define both the FHT_LEN and FHT_SCALE values to match according to the
// following table:
//
// FHT_SCALE | FHT_LEN
//     5     |   32
//     6     |   64
//     7     |  128
//     8     |  256

// Define the required FHT length here (i.e. the number of input samples).
// Valid values are 32, 64, 128 or 256.
#define FHT_LEN		256
#define FHT_SCALE	8

// Define the required output range of the linear complex number
// to real dB value output scaling here.
// Valid values are 64 (output range 0-64) or 128 (output range
// 0-128)
#define N_DB 64

// Uncomment the following line to make the library use
// PROGMEM (on AVR microcontrollers), otherwise the library
// will use local RAM for all look-up tables.  Uncommenting
// this define also includes the required headers for AVR
// compilation.

//#define AVR_GCC

// If you want to include streaming debug via printf uncomment
// the following line.  Note: if you are targeting an AVR uC
// you will need to include appropriate functions to allow
// streaming of printf via the USART for this to work:
//#define PRINTF_DEBUG

// If you want *lots* of debug from the FHT routine uncomment
// the PRINTF_DEBUG line AND the following line
//#define FHT_DEBUG

// Uncomment the next line if you want to include the pre-calculated
// table generation code.  This is only required if you want to change
// the table calculations - then you need to run the tableGenerate()
// function and copy-paste the tables back into the FHT library code.
// Note: This requires the math.h library which is included automatically
// if you uncomment the following define.  It also sets the PRINTF_DEBUG
// flag as the tables require stdio output to work
//
// If your target device has low-RAM (like the ATmega328P) you may need
// to set the FHT_LEN to the lowest setting to make the table generation
// compile as it needs a lot of RAM space due to the floating-point
// operations.
//#define GENERATE_TABLES

// No need to edit below this line -----------------------------------------------------

// If we are not targeting AVR GCC make a dummy define for PROGMEM
// (causes all tables to be stored in RAM instead)
#ifndef AVR_GCC
#define PROGMEM
#else
#include <avr/io.h>
#include <avr/pgmspace.h>
#endif

// If we are including the table generation code we need
// to include the math.h include (depending on your IDE
// you may need to add -lm to the linker stage to make
// this work.
#ifdef GENERATE_TABLES
#include <math.h>
// include PRINTF_DEBUG if not already defined
#ifndef PRINTF_DEBUG
#define PRINTF_DEBUG
#endif
#endif

// If we are using printf to output debug then we need to
// include stdio.h
#ifdef PRINTF_DEBUG
#include <stdio.h>
#endif

// Include the standard int and bool type definitions
#include <stdbool.h>
#include <stdint.h>

// Include all of the FHT functions
#ifdef GENERATE_TABLES
#include "generateTables.h"
#include "fhtLibraryVersion.h"
#include "fhtDebug.h"
#else
#include "fhtLibraryVersion.h"
#include "windowing.h"
#include "fht.h"
#include "complexToReal.h"
#include "complexToDecibel.h"
#include "fhtDebug.h"
#endif

#endif /* FHTCONFIG_H_ */

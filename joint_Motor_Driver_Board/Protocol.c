/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Pavlo Milo Manovi, Jeffrey Friesen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file	Protocol.c
 * @author 	Pavlo Manovi
 * @author 	Jeffrey Friesen
 * @date	September, 2014
 * @brief 	This library provides approximate NMEA-0183 method implementation.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Protocol.h"

//for debug
#include <uart.h>

enum {
	STATE_WAITING,
	STATE_RECORDING,
	STATE_CHECKSUM_ONE,
	STATE_CHECKSUM_TWO,
	STATE_CR,
	STATE_LF
};

typedef struct {
	uint8_t currentState;
	NMEA_0183_Sentence data;
	StringCommand *out;
	uint8_t index;
} NMEA_INFO;

uint8_t inited = 0;
static NMEA_INFO nmea;
uint8_t empty[64] = {};

uint8_t Hex2Dec(uint8_t newChar);

void InitDecoder(StringCommand *out)
{
	if (!inited) {
		nmea.out = out;
		nmea.currentState = STATE_WAITING;
		nmea.index = 0;
		inited = 1;
	}
}

/**
 * @brief Builds and checks a sentence from a character stream.
 * @param newChar New character in the received sentence.
 * @return NULL unless the sentence has been fully parsed.
 */
uint8_t DecodeStream(uint8_t newChar)
{

	switch (nmea.currentState) {
	case STATE_WAITING:
		if (newChar == '$') {
			memcpy(nmea.data.sentence, empty, sizeof(empty));
			nmea.currentState = STATE_RECORDING;
			return(0);
		} else {
			nmea.currentState = STATE_WAITING;
			return(0);
		}
		break;

	case STATE_RECORDING:
		if ((newChar == '$') || ((newChar == '*' && (nmea.index < 4))) ||
			newChar == '\r' || newChar == '\n') {
			nmea.index = 0;
			nmea.currentState = STATE_WAITING;
			return(0);
		} else if (newChar == '*') {
			nmea.currentState = STATE_CHECKSUM_ONE;
			return(0);
		} else {
			nmea.data.sentence[nmea.index] = newChar;
			nmea.index += 1;
			return(0);
		}
		break;

	case STATE_CHECKSUM_ONE:
		if (newChar == '$' || newChar == '*' || newChar == '\r' || newChar == '\n') {
			nmea.index = 0;
			nmea.currentState = STATE_WAITING;
			return(0);
		} else {
			nmea.data.checksum = Hex2Dec(newChar) << 4;
			nmea.currentState = STATE_CHECKSUM_TWO;
			return(0);
		}
		break;

	case STATE_CHECKSUM_TWO:
		if (newChar == '$' || newChar == '*' || newChar == '\r' || newChar == '\n') {
			nmea.index = 0;
			nmea.currentState = STATE_WAITING;
			return(0);
		} else {
			nmea.data.checksum |= Hex2Dec(newChar);
			nmea.currentState = STATE_CR;
			return(0);
		}
		break;

	case STATE_CR:
		if (newChar == '$' || newChar == '*' || newChar == '\n') {
			nmea.index = 0;
			nmea.currentState = STATE_WAITING;
			return(0);
		} else {
			nmea.currentState = STATE_LF;
			return(0);
		}
		break;

	case STATE_LF:
		if (newChar == '$' || newChar == '*' || newChar == '\r') {
			nmea.index = 0;
			nmea.currentState = STATE_WAITING;
			return(0);
		} else {
			uint16_t i = 0;
			uint16_t checkSum = 0;
			uint8_t floatBuff[16];

			//Verify checksum here.
			while (i < nmea.index) {
				checkSum ^= nmea.data.sentence[i];
				i++;
			}

			if (checkSum == nmea.data.checksum) {
				nmea.index = 0;
				nmea.currentState = STATE_WAITING;
				nmea.out->length1 = atoi(strtok((char *) nmea.data.sentence,(const char *) ","));
				nmea.out->length2 = atoi(strtok(NULL, (const char *) ","));
				nmea.out->length3 = atoi(strtok(NULL, (const char *) ","));
				nmea.out->length4 = atoi(strtok(NULL, (const char *) ","));
                                nmea.out->LAcommand = atoi(strtok(NULL, (const char *) ","));
				return(1);
			} else {
				nmea.index = 0;
				nmea.currentState = STATE_WAITING;
				return(0);
			}
		}
		break;

	default:
		return(0);
		break;
	}
}

/**
 * @brief Packs data into the NMEA-0183 format to be sent over serial comms.
 * @param data Takes a StringData type.
 * @return NULL if there are any errors.  Pointer to sentence otherwise.
 */
uint8_t *BuildSentence(StringData data)
{
	uint8_t i = 1;
	uint8_t temp[32];
	uint16_t checkSum = 0;
	uint8_t zero[1];
	zero[0] = 0;

	sprintf((char *) nmea.data.sentence, "$%f,%f,%f,%f", (float) data.length1, (float) data.length2,
		(float) data.length3, (float) data.length4);

	while (i < strlen((char *) nmea.data.sentence)) {
		checkSum ^= nmea.data.sentence[i];
		i++;
	}

	sprintf((char *) temp, "*%X\r\n", checkSum);
	strcat((char *) nmea.data.sentence, (char *) temp);

	return((uint8_t *) nmea.data.sentence);
}


uint8_t Hex2Dec(uint8_t halfhex)
{
	char rv;
	// Test for numeric characters
	if ((rv = halfhex - '0') <= 9 && rv >= 0) {
		return rv;
	}// Otherwise check for upper-case A-F
	else if ((rv = halfhex - 'A') <= 5 && rv >= 0) {
		return rv + 10;
	}// Finally check for lower-case a-f
	else if ((rv = halfhex - 'a') <= 5 && rv >= 0) {
		return rv + 10;
	}
	// Otherwise return -1 as an error
	return -1;
}

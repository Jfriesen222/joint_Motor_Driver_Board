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
 * @file	Protocol.h
 * @author 	Pavlo Manovi
 * @author 	Jeffrey Friesen
 * @date	September, 2014
 * @brief 	This library provides approximate NMEA-0183 comm methods.
 *
 */

#ifndef PROTOCOL_H
#define	PROTOCOL_H

#include <stdint.h>

typedef struct {
    uint8_t sentence[64];
    uint16_t checksum;
} NMEA_0183_Sentence;

typedef struct {
    int length1;
    int length2;
    int length3;
    int length4;
    int LAcommand;
} StringCommand;

typedef StringCommand StringData;


void InitDecoder(StringCommand *out);

/**
 * @brief Builds and checks a sentence from a character stream.
 * @param newChar New character in the received sentence.
 * @return NULL unless the sentence has been fully parsed. 1 when complete.
 */
uint8_t DecodeStream(uint8_t newChar);

/**
 * @brief Packs data into the NMEA-0183 format to be sent over serial comms.
 * @param data Takes a StringData type.
 * @return NULL if there are any errors.  Pointer to sentence otherwise.
 */
uint8_t *BuildSentence(StringData data);

#endif	/* PROTOCOL_H */


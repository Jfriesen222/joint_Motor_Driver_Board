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
#include "NMEAProtocol.h"
#include "Motor_Control_Board.h"

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
    Commands *out;
    uint8_t index;
} NMEA_INFO;


uint8_t inited = 0;
static NMEA_INFO nmea_in;

uint8_t empty[64] = {};

uint8_t Hex2Dec(uint8_t newChar);

void InitDecoder(Commands *out) {
    if (!inited) {
        nmea_in.out = out;
        nmea_in.currentState = STATE_WAITING;
        nmea_in.index = 0;
        inited = 1;
    }
}

/**
 * @brief Builds and checks a sentence from a character stream.
 * @param newChar New character in the received sentence.
 * @return NULL unless the sentence has been fully parsed.
 */
uint8_t DecodeStream(uint8_t newChar) {

    switch (nmea_in.currentState) {
        case STATE_WAITING:
            if (newChar == '$') {
                memcpy(nmea_in.data.sentence, empty, sizeof (empty));
                nmea_in.currentState = STATE_RECORDING;
                return (0);
            } else {
                nmea_in.currentState = STATE_WAITING;
                return (0);
            }
            break;

        case STATE_RECORDING:
            if ((newChar == '$') || ((newChar == '*' && (nmea_in.index < 20))) ||
                    newChar == '\r' || newChar == '\n') {
                nmea_in.index = 0;
                nmea_in.currentState = STATE_WAITING;
                return (0);
            } else if (newChar == '*') {
                nmea_in.currentState = STATE_CHECKSUM_ONE;
                return (0);
            } else {
                nmea_in.data.sentence[nmea_in.index] = newChar;
                nmea_in.index += 1;
                return (0);
            }
            break;

        case STATE_CHECKSUM_ONE:

            if (newChar == '$' || newChar == '*' || newChar == '\r' || newChar == '\n') {
                nmea_in.index = 0;
                nmea_in.currentState = STATE_WAITING;
                return (0);
            } else {
                nmea_in.data.checksum = Hex2Dec(newChar) << 4;
                nmea_in.currentState = STATE_CHECKSUM_TWO;
                return (0);
            }
            break;

        case STATE_CHECKSUM_TWO:
            if (newChar == '$' || newChar == '*' || newChar == '\r' || newChar == '\n') {
                nmea_in.index = 0;
                nmea_in.currentState = STATE_WAITING;
                return (0);
            } else {
                nmea_in.data.checksum |= Hex2Dec(newChar);
                nmea_in.currentState = STATE_CR;
                return (0);
            }
            break;

        case STATE_CR:
            if (newChar == '$' || newChar == '*' || newChar == '\n') {
                nmea_in.index = 0;
                nmea_in.currentState = STATE_WAITING;
                return (0);
            } else {
                nmea_in.currentState = STATE_LF;
                return (0);
            }
            break;

        case STATE_LF:
            if (newChar == '$' || newChar == '*' || newChar == '\r') {
                nmea_in.index = 0;
                nmea_in.currentState = STATE_WAITING;
                return (0);
            } else {
                uint16_t i = 0;
                uint8_t checkSum = 0;

                //Verify checksum here.
                while (i < nmea_in.index) {
                    checkSum ^= nmea_in.data.sentence[i];
                    i++;
                }
                long int datas[3];
                if (nmea_in.index == 21 && checkSum == nmea_in.data.checksum) {
                    for (i = 0; i < 2; i++) {
                        datas[i] = 0;
                        datas[i] = (255 | datas[i]) << 8;
                        datas[i] = (nmea_in.data.sentence[3 * i + 2] | datas[i]) << 8;
                        datas[i] = (nmea_in.data.sentence[3 * i + 1] | datas[i]) << 8;
                        datas[i] = (nmea_in.data.sentence[3 * i] | datas[i]);
                    }
                    datas[2] = 0;
                    datas[2] = (nmea_in.data.sentence[8] | datas[2]) << 8;
                    datas[2] = (nmea_in.data.sentence[7] | datas[2]) << 8;
                    datas[2] = (nmea_in.data.sentence[6] | datas[2]);
                    nmea_in.out->cmd1 = datas[0];
                    nmea_in.out->cmd2 = datas[1];
                    nmea_in.out->cmd3 = datas[2];
                    nmea_in.index = 0;
                    return (1);
                } else {
                    nmea_in.index = 0;
                    nmea_in.currentState = STATE_WAITING;
                    return (0);
                }
            }
            break;

        default:
            return (0);
            break;
    }
}

/**
 * @brief Packs data into the NMEA-0183 format to be sent over serial comms.
 * @param data Takes a StringData type.
 * @return NULL if there are any errors.  Pointer to sentence otherwise.
 */
void BuildSentence(uint8_t *sentence, Robot_Encoders *encoders) {
    uint8_t i = 1;
    uint8_t temp[5];

    uint8_t checkSum = 0;
    uint8_t j;
    sentence[0] = '$';
    for (j = 0; j < 2; j++) {
        sentence[16 * j + 1] = (uint8_t) ((encoders->RL_ENCDR[j][0] & 0x000000FF));
        sentence[16 * j + 2] = (uint8_t) ((encoders->RL_ENCDR[j][0] & 0x0000FF00) >> 8);
        sentence[16 * j + 3] = (uint8_t) ((encoders->RL_ENCDR[j][0] & 0x00FF0000) >> 16);
        sentence[16 * j + 4] = (uint8_t) ((encoders->RL_ENCDR[j][0] & 0xFF000000) >> 24);
        sentence[16 * j + 5] = (uint8_t) ((encoders->SF_ENCDR[j][0] & 0x000000FF));
        sentence[16 * j + 6] = (uint8_t) ((encoders->SF_ENCDR[j][0] & 0xFF00) >> 8);
        sentence[16 * j + 7] = (uint8_t) ((encoders->SA_ENCDR[j][0] & 0x00FF));
        sentence[16 * j + 8] = (uint8_t) ((encoders->SA_ENCDR[j][0] & 0xFF00) >> 8);
        sentence[16 * j + 9] = (uint8_t) ((encoders->RL_VEL[j] & 0x000000FF));
        sentence[16 * j + 10] = (uint8_t) ((encoders->RL_VEL[j] & 0x0000FF00) >> 8);
        sentence[16 * j + 11] = (uint8_t) ((encoders->RL_VEL[j] & 0x00FF0000) >> 16);
        sentence[16 * j + 12] = (uint8_t) ((encoders->RL_VEL[j] & 0xFF000000) >> 24);
        sentence[16 * j + 13] = (uint8_t) ((encoders->SF_VEL[j] & 0x00FF));
        sentence[16 * j + 14] = (uint8_t) ((encoders->SF_VEL[j] & 0xFF00) >> 8);
        sentence[16 * j + 15] = (uint8_t) ((encoders->SA_VEL[j] & 0x00FF));
        sentence[16 * j + 16] = (uint8_t) ((encoders->SA_VEL[j] & 0xFF00) >> 8);
    }

    for (i = 1; i < 33; i++) {
        checkSum ^= sentence[i];
    }
    sentence[33] = '*';
    //sprintf((char *) temp, "%02X", checkSum);
    sentence[34] = checkSum;
    sentence[35] = '\r';
    sentence[36] = '\n';
}

void DecodeSentence(uint8_t *data) {
    int32_t commands[3];
    uint8_t checkSum = 0;

    //    sentence[12] = '*';
    //    sprintf((char *) temp, "%02X", checkSum);
    //    sentence[13] = temp[0];
    //    sentence[14] = '\r';
    //    sentence[15] = '\n';
    int jj;
    for (jj = 0; jj < 12; jj++) {
        checkSum ^= data[jj];
    }
    if (data[12] == '*' && data[13] == checkSum && data[14] == '\r' && data[15] == '\n') {
        for (jj = 0; jj < 3; jj++) {
            commands[jj] = data[4 * jj + 3] << 8;
            commands[jj] = (commands[jj] | data[4 * jj + 2]) << 8;
            commands[jj] = (commands[jj] | data[4 * jj + 1]) << 8;
            commands[jj] = (commands[jj] | data[4 * jj + 0]);
        }
        nmea_in.out->cmd1 = commands[0];
        nmea_in.out->cmd2 = commands[1];
        nmea_in.out->cmd3 = commands[2];
    }
}

uint8_t Hex2Dec(uint8_t halfhex) {
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

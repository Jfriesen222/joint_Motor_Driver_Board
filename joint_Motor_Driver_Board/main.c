/*
 * File:   Soft_Robot_Board
 * Authors: Pavlo Milo Manovi and Jeffrey Friesen
 *
 *
 */
#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <uart.h>
#include <math.h>
#include "Motor_Control_Board.h"
#include "DMA_Transfer.h"
#include "CircularBuffer.h"
#include "NMEAProtocol.h"
#include "LS7366R_SPI.h"
#include <p33Exxxx.h>

/* Control Gains */
#define MAX_RL -1000
#define FORCE_Kp 150
int POS_Kp = 350;
#define POS_Kd 2000

#define RECALIBRATION
/* Force constraints  */
#define MAX_FORCE 100
#define MIN_FORCE 5
long int SA_MAX_VEL = 2000;
long int SF_MAX_VEL = 1000;

CircularBuffer uartBuffer;
uint8_t uartBuf[64];
CircularBuffer canBuffer;
uint8_t canBuf[64];
CircularBuffer spiBuffer;
uint16_t spiBuf[64];

int EncCts;
int long EncCtsLong;
int RegData;

Robot_Encoders robot_encoders;
Robot_Switches robot_switches;

int spi_error_count = 0;

uint16_t events;
bool controller;
uint32_t iii = 0;
int jj = 1;
int jjj = 0;
actuatorCommands commandSet;
int STATE = 1;
int motorduty[2];

enum {
    WAITING_MSG_START,
    MSG_RECIEVING
};
int state_msg = WAITING_MSG_START;

enum {
    EVENT_UART_DATA_READY = 0x01,
    EVENT_REPORT_FAULT = 0x08,
    EVENT_UPDATE_SPEED = 0x10,
    EVENT_UPDATE_LENGTHS = 0x40
};

void EventChecker(void);
void manageEncoders(void);
void manageMotors(int *targetForce);
void PositionPD(int *targetForce);
long int targetPosition[6];

int main(void) {
    int delay;
    for (delay = 0; delay < 10000; delay++);
    commandSet.cmd1 = 0;
    commandSet.cmd2 = 0;
    commandSet.cmd3 = 350;

    static uint8_t out[500];
    static uint8_t size;
    int targetForce[2];
    targetForce[0] = 5;
    targetForce[1] = 5;
    CB_Init(&uartBuffer, uartBuf, 64);
    InitBoard(&uartBuffer, EventChecker);
    InitDecoder(&commandSet);

    targetPosition[0] = commandSet.cmd1;
    targetPosition[1] = commandSet.cmd2;

    config_spi_slow();
    for (delay = 0; delay < 10000; delay++);

    selectCS(ALL_CS_LOW);
    setQuadX4();
    selectCS(ALL_CS_HIGH);

    for (delay = 0; delay < 100; delay++);

    selectCS(SF1 & SF2 & SA1 & SA2);
    set2ByteMode();
    selectCS(ALL_CS_HIGH);
    for (delay = 0; delay < 100; delay++);

    selectCS(RL1 & RL2);
    writeDTRtoZerosLong();
    selectCS(ALL_CS_HIGH);
    for (delay = 0; delay < 100; delay++);

    selectCS(SF1 & SF2 & SA1 & SA2);
    writeDTRtoZeros();
    selectCS(ALL_CS_HIGH);
    for (delay = 0; delay < 100; delay++);

    selectCS(ALL_CS_LOW);
    setCNTRtoDTR();
    selectCS(ALL_CS_HIGH);
    for (delay = 0; delay < 100; delay++);


    /* Set encoder counters to the quadrature mode */
#ifdef RECALIBRATION
    //
    //    if (checkSPIbus() != 0) {
    //        haltAndCatchFire((unsigned int *) "SPI bus failure\r\n");
    //    }
    //    /*check to make sure all of the limits switches are down, if not then hold and a make all of the motors make noise*/
    //    readSwitches(&robot_switches);
    //    if (robot_switches.SF[0] + robot_switches.SF[1] + robot_switches.SF[2] + robot_switches.SF[3] + robot_switches.SF[4] + robot_switches.SF[5] != 6) {
    //        haltAndCatchFire((unsigned int *) "Limit Switch not pressed, cables taught\r\n");
    //   }
#endif

    putsUART2((unsigned int *) "Init. Complete\r\n");

    while (1) {
        if (events & EVENT_UPDATE_SPEED) {
            iii++;
            /* main control loop*/
            // LED1 = 1;

            manageEncoders();
            // PositionPD(targetForce);
            manageMotors(targetForce);
            //   LED1 = 0;
//            if (STATE == 1) {
//                motorduty[0] += -1;
//                motorduty[1] += -1;
//
//            } else {
//                motorduty[0] += 1; //(CCW)Sets motor high to gpio pin 
//                motorduty[1] += 1;
//            }

            if (iii % 100 == 0) {
                LED1 = SW1; //(jj & 0b1);
                LED2 = SW2; //(jj & 0b10) >> 1;
                LED3 = SW3;(jj & 0b100) >> 2;
                jj = ((jj << 1));
                jj = (jj == 0b1000) ? 1 : jj;
//                if (iii % 4000 == 0) { //after 2000 counts, switches motor direction
//                    STATE = !STATE; //Changes to state from 0 to 1 or 1 to 0
//
//
//                    //MOTOR1 = STATE*PTPER; //sets duty state* pwm period on primary time base
//                    //MOTOR2 = STATE*PTPER;
//                }

                //                    size = sprintf((char *) out, "Step# %10lu RL: %10ld %10ld SF: %6d %6d SA: %6d %6d \r\n", iii,
                //                            robot_encoders.RL_ENCDR[0][0], robot_encoders.RL_ENCDR[1][0],
                //                            robot_encoders.SF_ENCDR[0][0], robot_encoders.SF_ENCDR[1][0],
                //                            robot_encoders.SA_ENCDR[0][0], robot_encoders.SA_ENCDR[1][0]);
                //                    DMA0_UART2_Transfer(size, out);

                //                int pos = 0;
                //                int spi_integrity;
                //                spi_integrity = checkSPIbus();
                //                if (spi_integrity != 0) {
                //                    if (spi_error_count >= 3) {
                //                        haltAndCatchFire((unsigned int *) "SPI bus failure\r\n");
                //                    } else {
                //                        spi_error_count++;
                //
                //                        size = sprintf((char *) out, "RL: %10ld %10ld SF: %6d %6d  SA: %6d %6d \r\n SPI ERROR %6d \r\n",
                //                                commandSet.cmd1, commandSet.cmd2,
                //                                robot_encoders.SF_ENCDR[0][pos], robot_encoders.SF_ENCDR[1][pos],
                //                                targetForce[0], targetForce[1], spi_integrity);
                //
                //                        DMA0_UART2_Transfer(size, out);
                //                    }
                //                } else {
                //                    spi_error_count = 0;




                //                size = sprintf((char *) out, "RL: %10ld %10ld SF: %6d %6d SA: %6d %6d\r\n",
                //                        robot_encoders.RL_VEL[0], robot_encoders.RL_VEL[1],
                //                        robot_encoders.SF_VEL[0], robot_encoders.SF_VEL[1],
                //                        robot_encoders.SA_VEL[0], robot_encoders.SA_VEL[1]);
                // size = sprintf((char *) out, "%lu \r\n",iii);
                //                                size = sprintf((char *) out, "1: %5i %5i %5i %5i 2: %5i %5i %5i %5i 3: %5i %5i %5i %5i \r\n",
                //                                        SW1_1, SW2_1, SW3_1, SW4_1,
                //                                        SW1_2, SW2_2, SW3_2, SW4_2,
                //                                        S_SF6, SW2_3, SW3_3, SW4_3);

                //DMA0_UART2_Transfer(size, out);
                //                }
            }
            events &= ~EVENT_UPDATE_SPEED;
        }

        if (events & EVENT_UART_DATA_READY) {

            events &= ~EVENT_UART_DATA_READY;
        }

        if (events & EVENT_REPORT_FAULT) {
            events &= ~EVENT_REPORT_FAULT;
        }

        if (events & EVENT_UPDATE_LENGTHS) {
            events &= ~EVENT_UPDATE_LENGTHS;
        }

        if (state_msg == WAITING_MSG_START) {

            if (uartBuffer.dataSize) {
                uint8_t data[1];
                CB_ReadByte(&uartBuffer, &data);
                //                DMA0_UART2_Transfer(1, data);
                if (data[0] == '$') {
                    state_msg = MSG_RECIEVING;
                }
            }//$12345678$01234567
        } else {
            if (uartBuffer.dataSize > 15) {
                uint8_t data2[16];
                CB_ReadMany(&uartBuffer, &data2, 16);
                DecodeSentence(&data2);
                //                data2[9] = 10; //carriage return
                //                //putsUART2(&data);
                //                //size = sprintf(out, "%c", data);
                //                DMA0_UART2_Transfer(10, data2);
                uint8_t sentence[37];
                CB_Remove(&uartBuffer, 30);
                //robot_encoders.RL_ENCDR[0][0] = commandSet.cmd2;
                //robot_encoders.RL_ENCDR[1][0] = commandSet.cmd3;
                BuildSentence(sentence, &robot_encoders);
                DMA0_UART2_Transfer(37, sentence);
                state_msg = WAITING_MSG_START;
            }
        }
    }
}

void EventChecker(void) {
    events |= EVENT_UPDATE_SPEED;
}

/* Reads encoders, resets them when switches are pressed, and calculates velocities*/
void manageEncoders() {
    uint16_t switchCS = 0;
    readSwitches(&robot_switches);
        switchCS = (robot_switches.SA[0] * ~SA1) | (robot_switches.SF[0] * ~SF1)
                | (robot_switches.SA[1] * ~SA2) | (robot_switches.SF[1] * ~SF2);

        selectCS(~switchCS);
        setCNTRtoDTR();
        selectCS(ALL_CS_HIGH);
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 1; j>-1; j--) {
            robot_encoders.RL_ENCDR[i][j + 1] = robot_encoders.RL_ENCDR[i][j];
            robot_encoders.SF_ENCDR[i][j + 1] = robot_encoders.SF_ENCDR[i][j];
            robot_encoders.SA_ENCDR[i][j + 1] = robot_encoders.SA_ENCDR[i][j];
        }
    }

    selectCS(RL1);
    readEncLong(&EncCtsLong);
    selectCS(ALL_CS_HIGH);
    robot_encoders.RL_ENCDR[0][0] = -EncCtsLong + (STRING_OFFSET_0 - STRING_ZERO_LENGTH);

    selectCS(RL2);
    readEncLong(&EncCtsLong);
    selectCS(ALL_CS_HIGH);
    robot_encoders.RL_ENCDR[1][0] = EncCtsLong + (STRING_OFFSET_1 - STRING_ZERO_LENGTH);

    selectCS(SF1);
    readEnc(&EncCts);
    selectCS(ALL_CS_HIGH);
    robot_encoders.SF_ENCDR[0][0] = EncCts; //-robot_encoders.SF_ENCDR[0][0] / 2 + EncCts;

    selectCS(SF2);
    readEnc(&EncCts);
    selectCS(ALL_CS_HIGH);
    robot_encoders.SF_ENCDR[1][0] = - EncCts;//-robot_encoders.SF_ENCDR[1][0] / 2 - EncCts;

    selectCS(SA1);
    readEnc(&EncCts);
    selectCS(ALL_CS_HIGH);
    robot_encoders.SA_ENCDR[0][0] = -EncCts;

    selectCS(SA2);
    readEnc(&EncCts);
    selectCS(ALL_CS_HIGH);
    robot_encoders.SA_ENCDR[1][0] = +EncCts;

    for (i = 0; i < 2; i++) {
        robot_encoders.SA_ENCDR[i][0] = robot_encoders.SA_ENCDR[i][0]*(1 - robot_switches.SA[i]);
        robot_encoders.SF_ENCDR[i][0] = robot_encoders.SF_ENCDR[i][0]*(1 - robot_switches.SF[i]);
    }
    int h = 250;
    for (i = 0; i < 2; i++) {
        robot_encoders.RL_VEL[i] = (1 * robot_encoders.RL_VEL[i]) / 2 + (3 * robot_encoders.RL_ENCDR[i][0] - 4 * robot_encoders.RL_ENCDR[i][1] + robot_encoders.RL_ENCDR[i][2]) * h;
        robot_encoders.SF_VEL[i] = (1 * robot_encoders.SF_VEL[i]) / 2 + (3 * robot_encoders.SF_ENCDR[i][0] - 4 * robot_encoders.SF_ENCDR[i][1] + robot_encoders.SF_ENCDR[i][2]) * h;
        robot_encoders.SA_VEL[i] = (1 * robot_encoders.SA_VEL[i]) / 2 + (3 * robot_encoders.SA_ENCDR[i][0] - 4 * robot_encoders.SA_ENCDR[i][1] + robot_encoders.SA_ENCDR[i][2]) * h;
    }
}

void manageMotors(int *targetForce) {
    int motorCommands[6];
    int jj = 0;

    for (jj = 0; jj < 2; jj++) {
        motorCommands[jj] =((robot_encoders.RL_ENCDR[jj][0] > MAX_RL) ? (-600) : (FORCE_Kp * (robot_encoders.SF_ENCDR[jj][0] - targetForce[jj])));
    }
    setMotors(motorCommands);
}

void PositionPD(int *targetForce) {
    targetPosition[0] = commandSet.cmd1;
    targetPosition[1] = commandSet.cmd2;
    POS_Kp = commandSet.cmd3;

    int jj = 0;
    for (jj = 0; jj < 2; jj++) {
        targetForce[jj] = (robot_encoders.RL_ENCDR[jj][0] - targetPosition[jj]) / POS_Kp; // + robot_encoders.RL_VEL[jj] / POS_Kd;
        targetForce[jj] = ((targetForce[jj] < MIN_FORCE) ? MIN_FORCE : (targetForce[jj] > MAX_FORCE) ? MAX_FORCE : targetForce[jj]);
    }
}

void __attribute__((__interrupt__, no_auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0; // Clear CN interrupt
}

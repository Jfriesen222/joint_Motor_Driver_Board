/* 
 * File:   PMSMBoard.h
 * Author: John Dean
 *
 * Created on August 8, 2016
 */

#include <xc.h>
#include "DMA_Transfer.h"
#include "CircularBuffer.h"

/* Motor PWM */
#define MOTOR1                PDC2                 
#define MOTOR2                PDC3

/* LEDs */
#define TRIS_LED1               TRISAbits.TRISA3
#define TRIS_LED2               TRISAbits.TRISA8
#define TRIS_LED3               TRISBbits.TRISB4

#define LED1                    LATAbits.LATA3
#define LED2                    LATAbits.LATA8
#define LED3                    LATBbits.LATB4



/* Motor Switches */
#define TRIS_SW1              TRISBbits.TRISB14
#define TRIS_SW2              TRISAbits.TRISA7
#define TRIS_SW3              TRISBbits.TRISB3
#define TRIS_SW4              TRISCbits.TRISC1
#define CNPU_SW1              CNPUBbits.CNPUB14
#define CNPU_SW2              CNPUAbits.CNPUA7
#define CNPU_SW3              CNPUBbits.CNPUB3
#define CNPU_SW4              CNPUCbits.CNPUC1
#define SW1                   PORTBbits.RB14
#define SW2                   PORTAbits.RA7
#define SW3                   PORTBbits.RB3
#define SW4                   PORTCbits.RC1
//
//#define S_SA1                     PORTAbits.RA6
//#define S_SF1                     PORTAbits.RA7
//#define S_SA2                     PORTAbits.RA4
//#define S_SF2                     PORTAbits.RA5
//
///* Chip Select */
#define TRIS_CS1             TRISBbits.TRISB15
#define TRIS_CS2             TRISBbits.TRISB2
#define TRIS_CS3              TRISAbits.TRISA0
#define TRIS_CS4              TRISAbits.TRISA10
#define TRIS_CS5              TRISCbits.TRISC0
#define TRIS_CS6              TRISAbits.TRISA1
#define TRIS_CS7              TRISCbits.TRISC7

#define CS1                   LATBbits.LATB15
#define CS2                   LATBbits.LATB2
#define CS3                   LATAbits.LATA0
#define CS4                   LATAbits.LATA10
#define CS5                   LATCbits.LATC0
#define CS6                   LATAbits.LATA1
#define CS7                   LATCbits.LATC7

#define ODC_CS1               ODCBbits.ODCB15
#define ODC_CS2               ODCBbits.ODCB2
#define ODC_CS3               ODCAbits.ODCA0
#define ODC_CS4               ODCAbits.ODCA10
#define ODC_CS5               ODCCbits.ODCC0
#define ODC_CS6               ODCAbits.ODCA1
#define ODC_CS7               ODCCbits.ODCC7

#define SF1                     0b1111111111111110
#define SF2                     0b1111111111111101
#define SA1                     0b1111111111111011
#define SA2                     0b1111111111110111
#define RL1                     0b1111111111101111
#define RL2                     0b1111111111011111
#define MD                      0b1111111110111111

#define ALL_CS_LOW              0x0
#define ALL_CS_HIGH             0xFF

/* Reset Pins */
#define TRIS_RESET_1            TRISAbits.TRISA2
#define RESET_1                 LATAbits.LATA2

#define PWMH_PIN_ENABLED        0b1000010000000000
#define PWML_PIN_ENABLED        0b0100010000000000

#define STRING_OFFSET_0     0//356700
#define STRING_OFFSET_1     0//347800
#define STRING_OFFSET_2     0//362800
#define STRING_OFFSET_3     0//352100
#define STRING_OFFSET_4     0//352800
#define STRING_OFFSET_5     0//356000
#define STRING_ZERO_LENGTH  0//330000




typedef struct{
    long int RL_ENCDR[2][3];
    int SF_ENCDR[2][3];
    int SA_ENCDR[2][3];
    long int RL_VEL[2];
    int SF_VEL[2];
    int SA_VEL[2];
} Robot_Encoders; 

typedef struct{
    uint8_t SF[2];
    uint8_t SA[2];
} Robot_Switches; 


void InitBoard(ADCBuffer *ADBuff, CircularBuffer *cB, CircularBuffer *spi_cB, void *eventCallback);
void selectCS(uint16_t cs_bits);
void readSwitches(Robot_Switches *robot_switches);
int checkSPIbus();
void haltAndCatchFire(unsigned int *message);
void setMotors(int *duty_cycle);
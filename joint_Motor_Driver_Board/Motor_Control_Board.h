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
//#define TRIS_SW1              TRISAbits.TRISA5
//#define TRIS_SW2              TRISAbits.TRISA4
//#define TRIS_SW3              TRISAbits.TRISA3
//#define TRIS_SW4              TRISAbits.TRISA2
//#define CNPU_SW1              CNPUAbits.CNPUA5
//#define CNPU_SW2              CNPUAbits.CNPUA4
//#define CNPU_SW3              CNPUAbits.CNPUA3
//#define CNPU_SW4              CNPUAbits.CNPUA2
//#define SW1                   PORTAbits.RA5
//#define SW2                   PORTAbits.RA4
//#define SW3                   PORTAbits.RA3
//#define SW4                   PORTAbits.RA2
//
//#define S_SA1                     PORTAbits.RA6
//#define S_SF1                     PORTAbits.RA7
//#define S_SA2                     PORTAbits.RA4
//#define S_SF2                     PORTAbits.RA5
//
///* Chip Select */
//#define TRIS_CS1_1              TRISDbits.TRISD0
//#define TRIS_CS2_1              TRISDbits.TRISD11
//#define TRIS_CS3_1              TRISDbits.TRISD10
//#define TRIS_CS4_1              TRISDbits.TRISD9
//#define TRIS_CS5_1              TRISDbits.TRISD8
//#define TRIS_CS6_1              TRISAbits.TRISA15
//#define CS1_1                   LATDbits.LATD0
//#define CS2_1                   LATDbits.LATD11
//#define CS3_1                   LATDbits.LATD10
//#define CS4_1                   LATDbits.LATD9
//#define CS5_1                   LATDbits.LATD8
//#define CS6_1                   LATAbits.LATA15
//#define ODC_CS1_1               ODCDbits.ODCD0
//#define ODC_CS2_1               ODCDbits.ODCD11
//#define ODC_CS3_1               ODCDbits.ODCD10
//#define ODC_CS4_1               ODCDbits.ODCD9
//#define ODC_CS5_1               ODCDbits.ODCD8
//#define ODC_CS6_1               ODCAbits.ODCA15

//#define RL1_1                     0b1111111111111110

//#define ALL_CS_LOW              0x0
//#define ALL_CS_HIGH             0xFF

//#define SF_ODD_1                SF3_1&SF5_1
//#define SF_ODD_2                SF1_2
//#define SF_EVEN_1               SF2_1&SF4_1&SF6_1 
//#define SF_EVEN_2               ALL_CS_HIGH
//#define SA_ODD_1                SA3_1&SA5_1
//#define SA_ODD_2                SA1_2
//#define SA_EVEN_1               SA2_1&SA4_1&SA6_1
//#define SA_EVEN_2               ALL_CS_HIGH
//#define RL_ODD_1                RL1_1&RL3_1&RL5_1
//#define RL_ODD_2                ALL_CS_HIGH
//#define RL_EVEN_1               RL2_1&RL4_1&RL6_1
//#define RL_EVEN_2               ALL_CS_HIGH


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
void selectCS(uint16_t cs_bits1, uint16_t cs_bits2 );
void readSwitches(Robot_Switches *robot_switches);
int checkSPIbus();
void haltAndCatchFire(unsigned int *message);
void setMotors(int *duty_cycle);
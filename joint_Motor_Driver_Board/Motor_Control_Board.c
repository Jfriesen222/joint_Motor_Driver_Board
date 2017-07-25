#include "Motor_Control_Board.h"
#include "LS7366R_SPI.h"
#include <timer.h>
#include <uart.h>
#include <math.h>
#include <pps.h>
#include <p33Exxxx.h>
#include <stdlib.h>

#pragma config FNOSC = FRC              // Oscillator Mode (Internal Fast RC (FRC))
#pragma config IESO = OFF                // Two-speed Oscillator Start-Up Enable (Start up with FRC, then switch)
#pragma config DMTEN = 0                // Dead man timer

#pragma config POSCMD = NONE            // Primary Oscillator Source (Primary Oscillator Disabled)
#pragma config OSCIOFNC = ON           // OSC2 Pin Function (OSC pin has I/O function)
#pragma config FCKSM = CSECMD 
//_FPOR(ALTI2C1_OFF)
// FWDT
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user so
#pragma config PWMLOCK = OFF
// FICD
#pragma config ICS = PGD3               // Comm Channel Select (Communicate on PGC1/EMUC1 and PGD1/EMUD1)

//#pragma config OSCIOFNC = OFF 

void UART2Init(void);
void ClockInit(void);
void MotorInit(void);
void TimersInit(void);
void PinInit(void);
void EventCheckInit(void *eventCallback);
void ADCInit(void);

#define PPSIn(fn,pin)     iPPSInput(IN_FN_PPS##fn,IN_PIN_PPS##pin)
#define PPSOut(fn,pin)    iPPSOutput(OUT_PIN_PPS##pin,OUT_FN_PPS##fn)

void (*eventCallbackFcn)(void);

void InitBoard(CircularBuffer *cB, void *eventCallback) {

    ClockInit();
    PinInit();
    MotorInit();
    UART2Init();


    DMA2REQbits.FORCE = 1;
    while (DMA2REQbits.FORCE == 1);
    
    //INTCON2bits.ALTIVT = 0

    DMA1_UART2_Enable_RX(cB);

    EventCheckInit(eventCallback);
    TimersInit();
}

void MotorInit() {

    /* Set PWM Period on Primary Time Base */
    PTPER = 2000;
    /* Set Duty Cycles */

    /* Set Dead Time Values */
    DTR2 = DTR3 =  0;
    ALTDTR2 = ALTDTR3 = 0;
    /* Set PWM Mode to Redundant */
     //independent >0xCC00; complementary = 0xC000; Redundant = 0xC400 
    IOCON1 = 0;
    IOCON2 = IOCON3 =  0xC400; 
    /* Set Primary Time Base, Edge-Aligned Mode and Independent Duty Cycles */
    PWMCON2 = PWMCON3 =  0x0000;
    /* Configure Faults */
    FCLCON2 = FCLCON3 =  0x0003;
    /* 1:1 Prescaler */
    PTCON2 = 0x0000;
    /* Enable PWM Module */
    PTCON = 0x8000;
    MOTOR1 = 0;
    MOTOR2 = 0;
}

void UART2Init(void) {
    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Auto-baud disabled
    U2MODEbits.BRGH = 1; // High speed UART mode...
    U2BRG = 18; //455 for 9600,227 for 19200, 113 for 38400,  37 for 115200 on BRGH 0, 460800 on BRGH 1, 921600 = 18 11 for 1500000 baud
    //BRGH = 0, BRG = 18 for 230400, BRG = 17 BRGH = 0 for 25000
    U2STAbits.UTXISEL0 = 0; // int on last character shifted out tx register
    U2STAbits.UTXISEL1 = 0; // int on last character shifted out tx register
    U2STAbits.URXISEL = 0; // Interrupt after one RX character is received
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART TX
    
    
    PPSOut(_U2TX, _RP40);// Connect UART2 TX output to RP40 pin
    PPSIn(_U2RX, _RP39); // Connect UART2 RX output to RP39 pin
    
}

void ClockInit(void) {
    // 140.03 MHz VCO  -- 70 MIPS
    PLLFBD = 74;
    CLKDIVbits.PLLPRE = 0;
    CLKDIVbits.PLLPOST = 0;
    OSCTUN = 0;                                 /* Tune FRC oscillator, if FRC is used */
    RCONbits.SWDTEN = 0;
    // Initiate Clock Switch to FRC oscillator with PLL (NOSC=0b001)
    __builtin_write_OSCCONH(0x01);

    __builtin_write_OSCCONL(OSCCON | 0x01);

    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b001);

    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1);
    
    ////////////////////////////////////////////////////////
//    
//    PLLFBD = 38;/* M=40 */
//    CLKDIVbits.PLLPOST = 0;                     /* N1=2 */
//    CLKDIVbits.PLLPRE = 0;                      /* N2=2 */
//    OSCTUN = 0;                                 /* Tune FRC oscillator, if FRC is used */
//
//    /* Disable Watch Dog Timer */
//    RCONbits.SWDTEN = 0;
//
//    /* Clock switch to incorporate PLL*/
//    __builtin_write_OSCCONH( 0x03 );            // Initiate Clock Switch to Primary
//
//    // Oscillator with PLL (NOSC=0b011)
//    __builtin_write_OSCCONL( OSCCON || 0x01 );  // Start clock switching
//    while( OSCCONbits.COSC != 0b011 );
//
//    // Wait for Clock switch to occur
//    /* Wait for PLL to lock */
//    while( OSCCONbits.LOCK != 1 )
//    { };
//}
}

void PinInit(void) {

    I2C1CON1 = 0;
    I2C1CON2 = 0;
    CM5CON = 0;

    //Right now no analog peripherals are being used, so we let digital
    //peripherals take over.
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    
    /* initialize LED pins as outputs */
    TRIS_LED1 = 0;
    TRIS_LED2 = 0;
    TRIS_LED3 = 0;
    
    /* initialize CS pins as outputs */
    TRIS_CS1 = TRIS_CS2 = TRIS_CS3 = TRIS_CS4 = TRIS_CS5 = TRIS_CS6 = TRIS_CS7 = 0;
    ODC_CS1 = ODC_CS2 = ODC_CS3 = ODC_CS4 = ODC_CS5 = ODC_CS6 = ODC_CS7 = 0;
    CS1 = CS2 = CS3 = CS4 = CS5 = CS6 =  1;
    CS7 = 1;

    /* initialize switch pins as inputs */
    TRIS_SW1 = 1;
    TRIS_SW2 = 1;
    TRIS_SW3 = 1;
    TRIS_SW4 = 1;
    CNPU_SW1 = 1;
    CNPU_SW2 = 1;
    CNPU_SW3 = 1;
    CNPU_SW4 = 1;

    TRIS_RESET_1 = 0;
    RESET_1 = 0                                                      ;

    //    //Set up Change Notify Interrupt
    //    CNENCbits.CNIEC13 = 1; // Enable RC14 pin for interrupt detection

//    //CNPUFbits.CNPUF4 = 1;
//    ODCFbits.ODCF4 = 0;
//    ODCFbits.ODCF5 = 0;
//    //CNPUCbits.CNPUC13 = 1;
//    IEC1bits.CNIE = 1; // Enable CN interrupts
//    IFS1bits.CNIF = 0; // Reset CN interrupt
    
//    TRISDbits.TRISD6 = 0;
//    TRISDbits.TRISD1 = 0;
//    TRISDbits.TRISD13 = 1; //sdi
//
//    TRISFbits.TRISF3 = 0;
//    TRISFbits.TRISF4 = 0;
//    TRISFbits.TRISF5 = 1; //sdi
//    
//    TRISDbits.TRISD5 = 1;
//    ODCDbits.ODCD5 = 0;
//    CNPUDbits.CNPUD5 = 0;

    /* Remap pins for SPI */
    PPSOut(_SDO2, _RP54); // Connect UART1 TX output to RP67 pin
    PPSOut(_SCK2, _RP41); // Connect Comparator 1 output to RP118 pin
    PPSIn(_SCK2, _RP41); // Set SCK remapped to input as well
    PPSIn(_SDI2, _RP56); // Connect UART2 RX input to RPI62 pin
}

void TimersInit(void) {
    T1CONbits.TON = 0;
    T1CONbits.TCS = 0;
    T1CONbits.TGATE = 0;
    T1CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
    TMR1 = 0x00;
    PR1 = 0x0112; //Approximately 5kHz... 0x0112 For 1kHz  0x0027 for 5kHz
    IPC0bits.T1IP = 0x01;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;
}

void EventCheckInit(void *eventCallback) {
    eventCallbackFcn = eventCallback;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    eventCallbackFcn();
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
}

void selectCS(uint8_t cs_bits) {
    CS1 = ((0b00000001)&(cs_bits));
    CS2 = ((0b00000010)&(cs_bits)) >> 1;
    CS3 = ((0b00000100)&(cs_bits)) >> 2;
    CS4 = ((0b00001000)&(cs_bits)) >> 3;
    CS5 = ((0b00010000)&(cs_bits)) >> 4;
    CS6 = ((0b00100000)&(cs_bits)) >> 5;
}

void readSwitches(Robot_Switches *robot_switches) {
    robot_switches->SF[0] = SW1;
    robot_switches->SF[1] = SW3;
    robot_switches->SA[0] = SW2;
    robot_switches->SA[1] = SW4;
}

void setMotors(int *duty_cycle) {
    //duty cycle is -1000 to 1000 
    if(duty_cycle[0] > 0){
        IOCON2 = 0x8400;
    }
    if(duty_cycle[0] < 0){
        IOCON2 = 0x4400;
    }
    if(duty_cycle[1] > 0){
        IOCON3 = 0x4400;
    }
    if(duty_cycle[1] < 0){
        IOCON3 = 0x8400;
    }
    MOTOR1 = abs(duty_cycle[0]);
    MOTOR2 = abs(duty_cycle[1]);
}

void haltAndCatchFire(unsigned int *message) {
    //putsUART2(message);
    PTPER = 500;
    IOCON1 = 0;
    IOCON2 = IOCON3 = 0xC000;
    MOTOR1 = PTPER / 2;
    MOTOR2 = PTPER / 2;
    while (1);
}

int checkSPIbus() {
 //   tripSPIdata register_data;
 //   int quadrx4 = QUADRX4;
    
//    selectCS(RL_ODD_1, RL_ODD_2);
//    readCountMode(&register_data);
//    selectCS(ALL_CS_HIGH, ALL_CS_HIGH);
//    if (register_data.data1 != quadrx4){
//        return 1;
//    } else if (register_data.data2 != quadrx4) {
//        return 2;
//    } else if (register_data.data3 != quadrx4) {
//        return 3;
//    }
//    
//    selectCS(RL_EVEN_1, RL_EVEN_2);
//    readCountMode(&register_data);
//    selectCS(ALL_CS_HIGH, ALL_CS_HIGH);
//    if (register_data.data1 != quadrx4){
//        return 1;
//    } else if (register_data.data2 != quadrx4) {
//        return 2;
//    } else if (register_data.data3 != quadrx4) {
//        return 3;
//    }
//
//    selectCS(SF_ODD_1, SF_ODD_2);
//    readCountMode(&register_data);
//    selectCS(ALL_CS_HIGH, ALL_CS_HIGH);
//    if (register_data.data1 != quadrx4){
//        return 1;
//    } else if (register_data.data2 != quadrx4) {
//        return 2;
//    } else if (register_data.data3 != quadrx4) {
//        return 3;
//    }
//
//    selectCS(SF_EVEN_1, SF_EVEN_2);
//    readCountMode(&register_data);
//    selectCS(ALL_CS_HIGH, ALL_CS_HIGH);
//    if (register_data.data1 != quadrx4){
//        return 1;
//    } else if (register_data.data2 != quadrx4) {
//        return 2;
//    } else if (register_data.data3 != quadrx4) {
//        return 3;
//    }
//    
//    selectCS(SA_ODD_1, SA_ODD_2);
//    readCountMode(&register_data);
//    selectCS(ALL_CS_HIGH, ALL_CS_HIGH);
//    if (register_data.data1 != quadrx4){
//        return 1;
//    } else if (register_data.data2 != quadrx4) {
//        return 2;
//    } else if (register_data.data3 != quadrx4) {
//        return 3;
//    }
//    
//    selectCS(SA_EVEN_1, SA_EVEN_2);
//    readCountMode(&register_data);
//    selectCS(ALL_CS_HIGH, ALL_CS_HIGH);
//    if (register_data.data1 != quadrx4){
//        return 1;
//    } else if (register_data.data2 != quadrx4) {
//        return 2;
//    } else if (register_data.data3 != quadrx4) {
//        return 3;
//    }

    return 0;
}
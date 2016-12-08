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

#define PPSIn(fn,pin)    iPPSInput(IN_FN_PPS##fn,IN_PIN_PPS##pin)
#define PPSOut(fn,pin)    iPPSOutput(OUT_PIN_PPS##pin,OUT_FN_PPS##fn)

void (*eventCallbackFcn)(void);

void InitBoard(ADCBuffer *ADBuff, CircularBuffer *cB, CircularBuffer *spi_cB, void *eventCallback) {

    ClockInit();
    PinInit();
    MotorInit();
    UART2Init();
    // ADCInit();


    DMA2REQbits.FORCE = 1;
    while (DMA2REQbits.FORCE == 1);

    DMA1_UART2_Enable_RX(cB);
    //DMA3_SPI_Enable_RX(spi_cB);
   // DMA6_ADC_Enable(ADBuff);


    EventCheckInit(eventCallback);
    TimersInit();
}

void MotorInit() {

    /* Set PWM Period on Primary Time Base */
    PTPER = 1000;
    /* Set Duty Cycles */

    /* Set Dead Time Values */
    DTR1 = DTR2 = DTR3 =  0;
    ALTDTR1 = ALTDTR2 = ALTDTR3 = 0;
    /* Set PWM Mode to Complementary */
     IOCON1 = IOCON2 = IOCON3 =  0xC000;; //independent >0xCC00;
    /* Set Primary Time Base, Edge-Aligned Mode and Independent Duty Cycles */
    PWMCON2 = PWMCON3 =  0x0000;
    /* Configure Faults */
    FCLCON1 = FCLCON2 = FCLCON3 =  0x0003;
    /* 1:1 Prescaler */
    PTCON2 = 0x0000;
    /* Enable PWM Module */
    PTCON = 0x8000;
    MOTOR1 = 6500;
    MOTOR2 = 6500;
    PDC1 = 6500;

}

void UART2Init(void) {
    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Auto-baud disabled
    U2MODEbits.BRGH = 1; // High speed UART mode...
    U2BRG = 18; //455 for 9600,227 for 19200, 113 for 38400,  37 for 115200 on BRGH 0, 460800 on BRGH 1, 921600 = 19
    //BRGH = 0, BRG = 18 for 230400, BRG = 17 BRGH = 0 for 25000
    U2STAbits.UTXISEL0 = 0; // int on last character shifted out tx register
    U2STAbits.UTXISEL1 = 0; // int on last character shifted out tx register
    U2STAbits.URXISEL = 0; // Interrupt after one RX character is received
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART TX
    
    PPSOut(_U2TX, _RP40); // Connect UART2 TX output to RP40 pin
    PPSIn(_U2RX, _RP39); // Connect UART2 RX output to RP39 pin
}

void ClockInit(void) {
    // 140.03 MHz VCO  -- 70 MIPS
    PLLFBD = 74;
    CLKDIVbits.PLLPRE = 0;
    CLKDIVbits.PLLPOST = 0;
    // Initiate Clock Switch to FRC oscillator with PLL (NOSC=0b001)
    __builtin_write_OSCCONH(0x01);

    __builtin_write_OSCCONL(OSCCON | 0x01);

    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b001);

    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1);
}

void PinInit(void) {

    I2C1CON1 = 0;
    I2C1CON2 = 0;

    //Right now no analog peripherals are being used, so we let digital
    //peripherals take over.
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    

    /* initialize LED pins as outputs */
    TRIS_LED1 = 0;
    TRIS_LED2 = 0;
    TRIS_LED3 = 0;

    
//    /* initialize CS pins as outputs */
    TRIS_CS1 = TRIS_CS2 = TRIS_CS3 = TRIS_CS4 = TRIS_CS5 = TRIS_CS6 = TRIS_CS7 = 0;
    ODC_CS1 = ODC_CS2 = ODC_CS3 = ODC_CS4 = ODC_CS5 = ODC_CS6 = ODC_CS7 = 0;
    CS1 = CS2 = CS3 = CS4 = CS5 = CS6 = CS7 = 1;

//    /* initialize switch pins as inputs */
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

//
    PPSOut(_SDO2, _RP54); // Connect UART1 TX output to RP67 pin
    PPSOut(_SCK2, _RP41); // Connect Comparator 1 output to RP118 pin
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

    TMR5 = 0x0000;
    T5CONbits.TCKPS = 3;
    PR5 = 60000; //68; // Trigger ADC1at a rate of 4kHz
    IFS1bits.T5IF = 0; // Clear Timer5 interrupt
    IEC1bits.T5IE = 0; // Disable Timer5 interrupt
    T5CONbits.TON = 1; // Start Timer5

    T3CONbits.TON = 0;
    T3CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
    TMR3 = 0x0000;
    PR3 = 60000; // Trigger ADC1 every 125 ?s @ 40 MIPS
    IFS0bits.T3IF = 0; // Clear Timer3 interrupt
    IEC0bits.T3IE = 0; // Disable Timer3 interrupt
    T3CONbits.TON = 1;
}

void ADCInit(void) {
    //ANSELBbits.ANSB14 = 1; //AN14
    //ANSELBbits.ANSB15 = 1; //AN15
    //ANSELBbits.ANSB13 = 1; //AN13
    //ANSELAbits.ANSA7 = 1; //AN23

    //Setup ADC1 for Channel 0-3 sampling
    AD1CON2bits.VCFG = 0;
    AD1CON1bits.FORM = 0; //Data Output Format : Integer Output
    AD1CON1bits.SSRCG = 1; //
    //AD1CON1bits.SSRC = 0; //Sample Clock Source : PWM Generator 1 primary trigger compare ends sampling
    AD1CON1bits.SSRC = 4; //Sample Clock Source : GP Timer5 starts conversion
    AD1CON1bits.ASAM = 1; // Sampling begins immediately after conversion
    AD1CON1bits.AD12B = 1; // 12-bit ADC operation
    AD1CON1bits.SIMSAM = 0; // Samples channel 0;

    AD1CON2bits.BUFM = 0;
    AD1CON2bits.CSCNA = 1; // Scan CH0+ Input Selections during Sample A bit
    AD1CON2bits.CHPS = 0; // Converts CH0  //This got changed

    AD1CON3bits.ADRC = 0; // ADC clock is derived from systems clock

    /*
     * ADCS is the main clock multiplier.  The result should be >= 1.6uS.
     * In a 40MHz processor the base period is 25nS.  64 * 25nS = 1.6uS.
     */
    AD1CON3bits.ADCS = 448; // 112 * 14.3nS = 1.6uS -- Conversion Clock
    AD1CON4bits.ADDMAEN = 1; // DMA Enable

    //AD1CHS0: Analog-to-Digital Input Select Register
    AD1CHS0bits.CH0SA = 0; // MUXA +ve input selection (AIN0) for CH0
    AD1CHS0bits.CH0NA = 0; // MUXA -ve input selection (VREF- if 0 AN1 if 1) for CH0
    //AD1CHS123: Analog-to-Digital Input Select Register
    AD1CHS123bits.CH123NA = 0; // MUXA -ve input selection (VREF- if 0 AN1 if 1) for CH1

    //AD1CSSH/AD1CSSL: Analog-to-Digital Input Scan Selection Register
    AD1CSSL = 0x0000;
    AD1CSSH = 0x0000;
    //AD1CSSLbits.CSS1 = 1;  //PVDD MONITORING
    AD1CSSLbits.CSS15 = 1;
    AD1CSSLbits.CSS14 = 1;
    AD1CSSLbits.CSS13 = 1;
    //AD1CSSLbits.CSS9 = 1;
    //AD1CSSLbits.CSS11 = 1; 
    //AD1CSSLbits.CSS13 = 1;

    AD1CON1bits.ADDMABM = 0; // DMA buffers are built in scatter/gather mode
    AD1CON2bits.SMPI = 4; // 2 ADC buffers
    AD1CON4bits.DMABL = 0b110; // Allocate one word of buffer per input.
    IFS0bits.AD1IF = 0; // Clear Analog-to-Digital Interrupt Flag bit
    IEC0bits.AD1IE = 0; // Do Not Enable Analog-to-Digital interrupt
    AD1CON1bits.ADON = 1; // Turn on the ADC
}

void EventCheckInit(void *eventCallback) {
    eventCallbackFcn = eventCallback;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    eventCallbackFcn();
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
}

void selectCS(uint16_t cs_bits) {
    CS1 = (0b0000000000000001)&(cs_bits);
    CS2 = ((0b0000000000000010)&(cs_bits)) >> 1;
    CS3 = ((0b0000000000000100)&(cs_bits)) >> 2;
    CS4 = ((0b0000000000001000)&(cs_bits)) >> 3;
    CS5 = ((0b0000000000010000)&(cs_bits)) >> 4;
    CS6 = ((0b0000000000100000)&(cs_bits)) >> 5;
    CS7 = ((0b0000000001000000)&(cs_bits)) >> 6;
}

void readSwitches(Robot_Switches *robot_switches) {
//    robot_switches->SF[0] = S_SF1;
//    robot_switches->SF[1] = S_SF2;
//    robot_switches->SA[0] = S_SA1;
//    robot_switches->SA[1] = S_SA2;
}

void setMotors(int *duty_cycle) {
    //duty cycle is -1000 to 1000 
//    MOTOR1 = (duty_cycle[1] + abs(duty_cycle[1])) / 2;
//    MOTOR2 = (duty_cycle[2] + abs(duty_cycle[2])) / 2;
}

void haltAndCatchFire(unsigned int *message) {
    putsUART2(message);
    PTPER = 500;
    IOCON1 = IOCON2 = IOCON3 = 0xC000;
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
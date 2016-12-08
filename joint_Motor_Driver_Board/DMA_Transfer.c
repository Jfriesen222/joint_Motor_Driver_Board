#include <xc.h>
#include <dma.h>
#include <stdint.h>
#include "DMA_Transfer.h"
#include "CircularBuffer.h"
#include "Motor_Control_Board.h"

//Protects the fuction from being interrupted.
//CB is non re-entrant so this is a big deal.
#define INTERRUPT_PROTECT(x) { \
char saved_ipl; \
\
SET_AND_SAVE_CPU_IPL(saved_ipl,7); \
x; \
RESTORE_CPU_IPL(saved_ipl); } (void) 0;

CircularBuffer *uart_CB_Point;
CircularBuffer *spi_CB_Point;
CircularBuffer *can_CB_Point;

uint16_t SPI2RxBuffA[16];
uint16_t BufferB[8];
ADCBuffer *ADCBuffPoint;

void DMA0_UART2_Transfer(uint16_t size, uint8_t *SendBuffer)
{
	DMA0CONbits.SIZE = 1; //Byte
	DMA0CONbits.DIR = 1; //RAM-to-Peripheral
	DMA0CONbits.HALF = 0;
	DMA0CONbits.NULLW = 0; //Null write disabled
	DMA0CONbits.AMODE = 0; //Register Indirect with Post-Increment
	DMA0CONbits.MODE = 1;

	DMA0CNT = (size - 1);
	DMA0REQ = 0x001F; // Select UART2 transmitter
	DMA0PAD = (volatile uint16_t) & U2TXREG;
	DMA0STAL = (volatile uint16_t) SendBuffer; //This may need some tweaking~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	DMA0STAH = 0x0000;

	IFS0bits.DMA0IF = 0; // Clear DMA Interrupt Flag
	IEC0bits.DMA0IE = 1; // Enable DMA interrupt
	DMA0CONbits.CHEN = 1; // Enable DMA channel
	DMA0REQbits.FORCE = 1;
}

void DMA1_UART2_Enable_RX(CircularBuffer *cB)
{
	uart_CB_Point = cB;
	//DMA1CON = 0x0000; // Continuous, Ping-Pong Disabled, Periph-RAM
	DMA1CONbits.SIZE = 1; //Byte
	DMA1CONbits.DIR = 0; //Peripheral-to-RAM
	DMA1CONbits.HALF = 0;
	DMA1CONbits.NULLW = 0; //Null write disabled
	DMA1CONbits.AMODE = 1; //Register Indirect without Post-Increment
	DMA1CONbits.MODE = 0; //Continuous, Ping-Pong mode disabled


	DMA1CONbits.MODE = 0; // Continuous, Ping-Pong Disabled
	DMA1CONbits.AMODE = 0; // Register indirect mode, post-increment
	//DMA1CNT = 7; // Eight DMA requests
	DMA1CNT = 0; // One DMA requests
	DMA1REQ = 0x001E; // Select UART2 receiver
	DMA1PAD = (volatile uint16_t) & U2RXREG;
	DMA1STAL = &BufferB;
	DMA1STAH = 0x0000;

	IFS0bits.DMA1IF = 0; // Clear DMA interrupt
	IEC0bits.DMA1IE = 1; // Enable DMA interrupt
	DMA1CONbits.CHEN = 1; // Enable DMA channel
}

void DMA2_SPI_Transfer(uint16_t size, uint16_t *SendBuffer)
{
	DMA2CONbits.SIZE = 0; //Word wide transfer
	DMA2CONbits.DIR = 1; //RAM-to-Peripheral
	DMA2CONbits.HALF = 0;
	DMA2CONbits.NULLW = 0; //Null write disabled
	DMA2CONbits.AMODE = 0; //Register Indirect with Post-Increment
	DMA2CONbits.MODE = 01; //One-Shot, Ping-Pong modes disabled

	DMA2CNT = (size - 1); //16 transfers
	DMA2REQ = 0x0A;
	DMA2PAD = (volatile uint16_t) & SPI2BUF;
	DMA2STAL = (uint16_t) SendBuffer;
	DMA2STAH = 0;

	SPI2BUF = *SendBuffer;

	IFS1bits.DMA2IF = 0; // Clear DMA interrupt
	IEC1bits.DMA2IE = 1; // Enable DMA interrupt
	DMA2CONbits.CHEN = 1; // Enable DMA Channel
	//DMA2REQbits.FORCE = 1;
}

void DMA3_SPI_Enable_RX(CircularBuffer *cB)
{
	spi_CB_Point = cB;
	DMA3CONbits.SIZE = 0; //Word wide transfer
	DMA3CONbits.DIR = 0; //Peripheral-to-RAM
	DMA3CONbits.HALF = 0;
	DMA3CONbits.NULLW = 0; //Null write disabled
	DMA3CONbits.AMODE = 1; //Register Indirect without Post-Increment
	DMA3CONbits.MODE = 0; //Continuous, Ping-Pong mode disabled

	DMA3CNT = 0;
	DMA3REQ = 0x0A;
	DMA3PAD = (volatile uint16_t) & SPI2BUF;
	DMA3STAL = &SPI2RxBuffA;
	DMA3STAH = 0x0000;

	IFS2bits.DMA3IF = 0; // Clear DMA interrupt
	IEC2bits.DMA3IE = 1; // Enable DMA interrupt
	DMA3CONbits.CHEN = 1; // Enable DMA Channel
}


void __attribute__((__interrupt__, no_auto_psv)) _DMA0Interrupt(void)
{
	IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt Flag
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA1Interrupt(void)
{
	INTERRUPT_PROTECT(CB_WriteMany(uart_CB_Point, BufferB, 1, 0));
	IFS0bits.DMA1IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA2Interrupt(void)
{
	IFS1bits.DMA2IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA3Interrupt(void)
{
	//Think about a global interrupt disable here as CB is non reentrant...
	CB_WriteByte(spi_CB_Point, SPI2RxBuffA[0]);
	IFS2bits.DMA3IF = 0;
}

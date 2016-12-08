/*
 * File:   DMA_Transfer.h
 * Author: pavlo
 *
 * Created on July 18, 2014, 3:49 PM
 */

#ifndef DMA_TRANSFER_H
#define	DMA_TRANSFER_H

#include "CircularBuffer.h"
typedef struct {
	uint16_t Adc1Data[10];
        uint16_t newData;
} ADCBuffer;

void DMA0_UART2_Transfer(uint16_t size, uint8_t *SendBuffer);

void DMA1_UART2_Enable_RX(CircularBuffer *cB);

void DMA2_SPI_Transfer(uint16_t size, uint16_t *SendBuffer);

void DMA3_SPI_Enable_RX(CircularBuffer *cB);

#endif	/* DMA_TRANSFER_H */
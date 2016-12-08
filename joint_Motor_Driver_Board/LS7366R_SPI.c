/*
 * File:   LS7366R_SPI.c
 * Author: Jeff
 *
 * Created on August 10, 2016, 2:06 PM
 */


#include "xc.h"

#include "LS7366R_SPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <spi.h>
#include <dma.h>
#include <xc.h>

tripSPIdata cntData1;
tripSPIdata cntData2;
tripSPIdata cntData3;
tripSPIdata cntData4;

unsigned int config1slow = ENABLE_SCK_PIN & // Internal Serial Clock is Enabled
        ENABLE_SDO_PIN & // SDO2 pin is controlled by the module
        SPI_MODE16_OFF & // Communication is 0=8-bits, 1=16bits wide
        SPI_SMP_OFF & // Input Data is Sampled at the Middle of Data Output Time
        SPI_CKE_ON & // Transmit happens from active clock state to idle clock state
        SLAVE_ENABLE_OFF & // Slave Select Disabled
        CLK_POL_ACTIVE_HIGH & // Idle state for clock is low, active is high
        MASTER_ENABLE_ON & // Master Mode
        SEC_PRESCAL_3_1 &
        PRI_PRESCAL_16_1 // SPI CLK at 5MHz
        ;


// This contains the parameters to be configured in the SPIxCON2 register
unsigned int config2 = FRAME_ENABLE_OFF & // Frame SPI support Disable
        FIFO_BUFFER_DISABLE // FIFO buffer disabled
        ;

// This contains the parameters to be configured in the SPIxSTAT register
unsigned int config3 = SPI_ENABLE & // Enable module
        SPI_IDLE_CON & // Continue module operation in idle mode
        SPI_RX_OVFLOW_CLR // Clear receive overflow bit
        ;

void config_spi_slow() {
    CloseSPI2();
    OpenSPI2(config1slow, config2, config3);
}

void setQuadX4() {
    // Initialize encoder 1
    //    Clock division factor: 0
    //    Negative index input
    //    free-running count mode
    //    x4 quatrature count mode (four counts per quadrature cycle)
    // NOTE: For more information on commands, see datasheet
    write_SPI(WRITE_MDR0); // Write to MDR0      
    write_SPI(QUADRX4); // Configure to 4 byte mode
}

void set2ByteMode() {
    // Initialize encoder 1
    //    Clock division factor: 0
    //    Negative index input
    //    free-running count mode
    //    x4 quatrature count mode (four counts per quadrature cycle)
    // NOTE: For more information on commands, see datasheet
    write_SPI(WRITE_MDR1); // Write to MDR0      
    write_SPI(0x02); // Configure to 4 byte mode
}



void writeDTRtoZerosLong() {
    write_SPI(WRITE_DTR);
    // Load data
    write_SPI(0x00); // Highest order byte
    write_SPI(0x00);
    write_SPI(0x00);
    write_SPI(0x00); // lowest order byte
}

void writeDTRtoZeros() {
    write_SPI(WRITE_DTR);
    // Load data
    write_SPI(0x00); // Highest order byte
    write_SPI(0x00);
}


void setCNTRtoDTR() {
    write_SPI(LOAD_CNTR);
}

void readEncLong(EncoderCtsLong *EncVals) {
    // Initialize temporary variables for SPI read
    write_SPI(READ_CNTR); // Request count
    read_SPI(0x00, &cntData1); // Read highest order byte
    read_SPI(0x00, &cntData2);
    read_SPI(0x00, &cntData3);
    read_SPI(0x00, &cntData4); // Read lowest order byte

    // Calculate encoder count
    long int count_value;
    count_value = (cntData1.data1 << 8) + cntData2.data1;
    count_value = (count_value << 8) + cntData3.data1;
    count_value = (count_value << 8) + cntData4.data1;
    EncVals->cts1 = count_value;
}

void readEnc(EncoderCts *EncVals) {
    // Initialize temporary variables for SPI read
    write_SPI(READ_CNTR); // Request count
    read_SPI(0x00, &cntData1); // Read highest order byte
    read_SPI(0x00, &cntData2);

    // Calculate encoder count
    int count_value;
    count_value = (cntData1.data1 << 8) + cntData2.data1;
    EncVals->cts1 = count_value;
}

void readCountMode(tripSPIdata *count_mode){
    write_SPI(READ_MDR0);
    read_SPI(0x00, &cntData1);
    
    count_mode->data1 = cntData1.data1;
    
}

void read_SPI(int command, tripSPIdata *datas) {
    int bufVal;
    bufVal = SPI2BUF; // dummy read of the SPI1BUF register to clear the SPIRBF flag
    SPI2BUF = command; // write the data out to the SPI peripheral
    while ((!SPI2STATbits.SPIRBF) ); // wait for the data to be sent out
    datas->data1 = SPI2BUF;
}

void write_SPI(int command) {
    int bufVal;
    bufVal = SPI2BUF; // dummy read of the SPI1BUF register to clear the SPIRBF flag
    SPI2BUF = command; // write the data out to the SPI peripheral
    while ((!SPI2STATbits.SPIRBF)  ); // wait for the data to be sent out
    bufVal = SPI2BUF; // dummy read of the SPI1BUF register to clear the SPIRBF flag
}
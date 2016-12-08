/*
 * File:   MAG3110.h
 * Author: Jonathan Bruce
 *
 * Created on October 20, 2014, 5:33 PM
 */

#ifndef AK8975_H
#define	AK8975_H

// Standard headers
#include <stdint.h>
#include <stdbool.h>

// User headers
#include "I2CdsPIC.h"

typedef struct {
    int8_t  mag_X_msb;
    int8_t  mag_X_lsb;
    int16_t magX;
    int8_t  mag_Y_msb;
    int8_t  mag_Y_lsb;
    int16_t magY;
    int8_t  mag_Z_msb;
    int8_t  mag_Z_lsb;
    int16_t magZ;
    int8_t  die_temp;
}AK8975_Data;

#define MAG_DR_STATUS1		0x02
#define MAG_DR_STATUS2		0x09
#define MAG_CTRL		0x0A
#define MAG_ADDRESS  0x0C
#define MAG_XOUT_L   0x03
#define MAG_XOUT_H   0x04
#define MAG_YOUT_L   0x05
#define MAG_YOUT_H   0x06
#define MAG_ZOUT_L   0x07
#define MAG_ZOUT_H   0x08





/**
 * @brief Inits the MAG3110 and sets active
 * Initializes Auto Magnetic Sensor Read and Raw data output
 * Everything else is left as default
 *
 * @see pages 19-21 of MAG3110.pdf
 */
void AK8975_Init(void);

/**
 * Retrieves X axis data from the MAG3110
 *
 * @param sensorData
 */
void AK8975_GetXData(AK8975_Data* sensorData);

/**
 * Retrieves Y axis data from the MAG3110
 *
 * @param sensorData
 */
void AK8975_GetYData(AK8975_Data* sensorData);

/**
 * Retrieves Z axis data from the MAG3110
 *
 * @param sensorData
 */
void AK8975_GetZData(AK8975_Data* sensorData);

/**
 * Retrieves Z, Y, & X axis data from MAG3110 using bursting mode
 * Bursting allows for all the data retrieved to be on the same sample time
 *
 * @param sensorData
 */
void AK8975_Get3AxisData(AK8975_Data* sensorData);

/**
 * Retrieves temperature data from the MAG3110
 *
 * @param sensorData

 * Read the status register on the MAG3110 and checks to see if X axis data is ready
 *
 * @return True when X axis data is ready, false otherwise
 */

/**
 * Read the status register on the MAG3110 and checks to see if any axis data is ready
 *
 * @return
 */
bool AK8975_Is_ZYX_Data_Ready(void);

/**
 * Returns the full status register from the MAG3110
 *
 * @return The 8 bit value of the current status register
 */
//uint8_t MAG3110_Get_DR_STATUS(void);

/**
 * This function will set user defined offset corrections for each X, Y, Z axis
 * on the MAG3110
 *
 * @param X_offset
 * @param Y_offset
 * @param Z_offset
 */
//void MAG3110_Set_Offest_Correction(uint16_t X_offset, uint16_t Y_offset, uint16_t Z_offset);

/**
 *
 * @param reg_value
 */
void AK8975_Set_CTRL_REG(unsigned char reg_value);

/**
 *
 * @param reg_value
 */
//void MAG3110_Set_CTRL_REG2(unsigned char reg_value);

/**
 *
 * @param enabled
 */
//void MAG3110_SetActive(bool enabled);

/**
 *
 * @param enabled
 */
//void MAG3110_SetRaw(bool enabled);

#endif	/* MAG3110_H */


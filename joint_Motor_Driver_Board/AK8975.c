/*
 * File:   MAG3110.c
 * Author: Jonathan Bruce
 *
 * Created on October 21, 2014, 10:23 AM
 */

#include "AK8975.h"

void AK8975_Init(void) {
    // Set the output data rate to 80Hz, over sampling to 16, enable reading
    // all 16-bit values, set normal operation, and enter standby mode.
    AK8975_Set_CTRL_REG(0x01);

}

void AK8975_GetXData(AK8975_Data* sensorData) {
    sensorData->mag_X_lsb = I2C_ReadFromReg(MAG_ADDRESS, MAG_XOUT_L);
    sensorData->mag_X_msb = I2C_ReadFromReg(MAG_ADDRESS, MAG_XOUT_H);
    sensorData->magX = (((int16_t) sensorData->mag_X_msb) << 8) | sensorData->mag_X_lsb;
}

void AK8975_GetYData(AK8975_Data* sensorData) {
    // Have to read register MAG_OUT_X_MSB to insure data is recent and good
    //sensorData->mag_X_lsb = I2C_ReadFromReg(MAG_ADDRESS, MAG_XOUT_L);

    sensorData->mag_Y_lsb = I2C_ReadFromReg(MAG_ADDRESS, MAG_YOUT_L);
    sensorData->mag_Y_msb = I2C_ReadFromReg(MAG_ADDRESS, MAG_YOUT_H);

    sensorData->magY = (((int16_t) sensorData->mag_Y_msb) << 8) | sensorData->mag_Y_lsb;
}

void AK8975_GetZData(AK8975_Data* sensorData) {
    // Have to read register MAG_OUT_X_MSB to insure data is recent and good
    //sensorData->mag_X_lsb = I2C_ReadFromReg(MAG_ADDRESS, MAG_XOUT_L);

    sensorData->mag_Z_lsb = I2C_ReadFromReg(MAG_ADDRESS, MAG_ZOUT_L);
    sensorData->mag_Z_msb = I2C_ReadFromReg(MAG_ADDRESS, MAG_ZOUT_H);
    sensorData->magZ = (((int16_t) sensorData->mag_Z_msb) << 8) | sensorData->mag_Z_lsb;
}

void AK8975_Get3AxisData(AK8975_Data* sensorData) {
    // Read all 6 bytes of magnetometer data
   //I2C_WriteToReg(MAG_ADDRESS, MAG_CTRL, 0);
   //I2C_WriteToReg(MAG_ADDRESS, MAG_CTRL, 1);
    uint8_t data[8];
    I2C_ReadFromReg_Burst(MAG_ADDRESS, MAG_DR_STATUS1, data,8);
    if(data[0])
    {

    sensorData->mag_X_lsb = data[1];
    sensorData->mag_X_msb = data[2];
    sensorData->magX = (((int16_t) sensorData->mag_X_msb) << 8) | sensorData->mag_X_lsb;
    sensorData->mag_Y_lsb = data[3];
    sensorData->mag_Y_msb = data[4];
    sensorData->magY = (((int16_t) sensorData->mag_Y_msb) << 8) | sensorData->mag_Y_lsb;
    sensorData->mag_Z_lsb = data[5];
    sensorData->mag_Z_msb = data[6];
    sensorData->magZ = (((int16_t) sensorData->mag_Z_msb) << 8) | sensorData->mag_Z_lsb;
    }
}

bool AK8975_Is_ZYX_Data_Ready(void) {
    uint8_t temp = I2C_ReadFromReg(MAG_ADDRESS, MAG_DR_STATUS1);
    if (temp) {
        return true;
    } else {
        return false;
    }
}

void AK8975_Set_CTRL_REG(unsigned char reg_value) {
    //temp = I2C_ReadFromReg(MAG_ADDRESS, MAG_CTRL);

    // Checks if the mag is active
    // if true, put into standby and change the register then place back into active mode
    // else just load the register

        I2C_WriteToReg(MAG_ADDRESS, MAG_CTRL, reg_value);

}


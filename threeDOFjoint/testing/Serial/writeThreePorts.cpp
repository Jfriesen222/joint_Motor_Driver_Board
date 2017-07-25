#include <math.h>
#include <matrix.h>
#include <mex.h>
#include <stdint.h>
#include "c_serial.h"
static c_serial_port_t* m_port1;
static c_serial_port_t* m_port2;
static c_serial_port_t* m_port3;
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if(nrhs!=6) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nrhs","Please send 3 serial port objects and 3 msgs.");
    }
    if(nlhs!=0) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nlhs","No outputs here.");
    }
    int status;
    int bytes_read;
    int data_length1, data_length2, data_length3;
    double *dataDouble1, *dataDouble2, *dataDouble3;
    uint8_t data1[50], data2[50], data3[50];
    dataDouble1 = mxGetPr(prhs[3]);
    data_length1 = mxGetN(prhs[3]);
    dataDouble2 = mxGetPr(prhs[4]);
    data_length2 = mxGetN(prhs[4]);
    dataDouble3 = mxGetPr(prhs[5]);
    data_length3 = mxGetN(prhs[5]);
    int i;
    for(i=0; i<data_length1; i++){
        data1[i] = (uint8_t) dataDouble1[i];
        data2[i] = (uint8_t) dataDouble2[i];
        data3[i] = (uint8_t) dataDouble3[i];
    }
    m_port1 = reinterpret_cast<c_serial_port_t*>(*((uint64_t *)mxGetData(prhs[0])));
    m_port2 = reinterpret_cast<c_serial_port_t*>(*((uint64_t *)mxGetData(prhs[1])));
    m_port3 = reinterpret_cast<c_serial_port_t*>(*((uint64_t *)mxGetData(prhs[2])));
    status = c_serial_write_data( m_port1, data1, &data_length1);
    if( status < 0 ){
        mexPrintf("error writing to port 1");
    }
    status = c_serial_write_data( m_port2, data2, &data_length2);
    if( status < 0 ){
        mexPrintf("error writing to port 2");
    }
    status = c_serial_write_data( m_port3, data3, &data_length3);
    if( status < 0 ){
        mexPrintf("error writing to port 3");
    }
}
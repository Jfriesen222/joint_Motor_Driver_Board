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
    if(nrhs!=3) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nrhs","Please send 3 serial port objects.");
    }
    if(nlhs!=3) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nlhs","3 serial data outputs required.");
    }
    int status;
    uint8_t data[37];
    int data_length;
    double *msgData;               /* output  vector */

    m_port1 = reinterpret_cast<c_serial_port_t*>(*((uint64_t *)mxGetData(prhs[0])));
    m_port2 = reinterpret_cast<c_serial_port_t*>(*((uint64_t *)mxGetData(prhs[1])));
    m_port3 = reinterpret_cast<c_serial_port_t*>(*((uint64_t *)mxGetData(prhs[2])));
    
    data_length = 37; 
    status = c_serial_read_data( m_port1, data, &data_length,  NULL);
    if( status < 0 ){
        mexPrintf( "Read Failed\n");
    }
    /* create the output matrix */
    plhs[0] = mxCreateDoubleMatrix(1,(mwSize)data_length,mxREAL);
    msgData = mxGetPr(plhs[0]);
    int j;
    for (j = 0; j < data_length; j++)
			msgData[j] = (double) data[j];    
        status = c_serial_read_data( m_port2, data, &data_length,  NULL);
    if( status < 0 ){
        mexPrintf( "Read Failed\n");
    }
    /* create the output matrix */
    plhs[1] = mxCreateDoubleMatrix(1,(mwSize)data_length,mxREAL);
    msgData = mxGetPr(plhs[1]);
    for (j = 0; j < data_length; j++)
			msgData[j] = (double) data[j];    
        status = c_serial_read_data( m_port3, data, &data_length,  NULL);
    if( status < 0 ){
        mexPrintf( "Read Failed\n");
    }
    /* create the output matrix */
    plhs[2] = mxCreateDoubleMatrix(1,(mwSize)data_length,mxREAL);
    msgData = mxGetPr(plhs[2]);
    for (j = 0; j < data_length; j++)
			msgData[j] = (double) data[j];    
}
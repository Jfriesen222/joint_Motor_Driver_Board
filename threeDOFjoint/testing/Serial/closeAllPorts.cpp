#include <math.h>
#include <matrix.h>
#include <mex.h>
#include <stdint.h>
#include "c_serial.h"
static c_serial_port_t* m_port;
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    const char** allPortList;
    allPortList = c_serial_get_serial_ports_list();
    c_serial_free_serial_ports_list(allPortList);
}




// 
// template<class base> inline class_handle<base> *convertMat2HandlePtr(const mxArray *in)
// {
//     if (mxGetNumberOfElements(in) != 1 || mxGetClassID(in) != mxUINT64_CLASS || mxIsComplex(in))
//         mexErrMsgTxt("Input must be a real uint64 scalar.");
//     class_handle<base> *ptr = reinterpret_cast<class_handle<base> *>(*((uint64_t *)mxGetData(in)));
//     if (!ptr->isValid())
//         mexErrMsgTxt("Handle not valid.");
//     return ptr;
// }

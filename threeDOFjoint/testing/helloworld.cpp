#include <math.h>
#include <matrix.h>
#include <mex.h>
#include <stdint.h>
#include "c_serial.h"
static c_serial_port_t* m_port;
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    mexPrintf("Hello World!\n");
    

    c_serial_control_lines_t m_lines;
    int status;
    int bytes_read;
    uint8_t data[ 255 ];
    int data_length;
    int x;
    
    /*
     * Use the first argument as the port to open
     */
//     if( argc != 2 ){
//         mexPrintf("ERROR: First argument must be serial port\n" );
//         /*
//          * Since no port was provided, print the available ports
//          */
//         const char** port_list = c_serial_get_serial_ports_list();
//         x = 0;
//         printf("Available ports:\n");
//         while( port_list[ x ] != NULL ){
//             printf( "%s\n", port_list[ x ] );
//             x++;
//         }
//         c_serial_free_serial_ports_list( port_list );
//         return 1;
//     }
    
    /*
     * Set the global log function.  This can also be set per-port
     * We will use a simple log function that prints to stderr
     */
    //c_serial_set_global_log_function( c_serial_stderr_log_function );
    
    /*
     * Allocate the serial port struct.
     * This defaults to 9600-8-N-1
     */
    if( c_serial_new( &m_port, NULL ) < 0 ){
        mexPrintf("ERROR: Unable to create new serial port\n" );
        return;
    }
//
//     /*
//      * The port name is the device to open(/dev/ttyS0 on Linux,
//      * COM1 on Windows)
//      */
    if( c_serial_set_port_name( m_port, "COM11" ) < 0 ){
        mexPrintf("ERROR: can't set port name\n" );
    }
//
//     /*
//      * Set various serial port settings.  These are the default.
//      */
    c_serial_set_baud_rate( m_port, CSERIAL_BAUD_921600 );
    c_serial_set_data_bits( m_port, CSERIAL_BITS_8 );
    c_serial_set_stop_bits( m_port, CSERIAL_STOP_BITS_1 );
    c_serial_set_parity( m_port, CSERIAL_PARITY_NONE );
    c_serial_set_flow_control( m_port, CSERIAL_FLOW_NONE );
//
    mexPrintf( "Baud rate is %d\n", c_serial_get_baud_rate( m_port ) );
//
//     /*
//      * We want to get all line flags when they change
//      */
    c_serial_set_serial_line_change_flags( m_port, CSERIAL_LINE_FLAG_ALL );
//
    status = c_serial_open( m_port );
    if( status < 0 ){
        mexPrintf("ERROR: Can't open serial port\n" );
        return;
    }
//
//     /*
//      * Listen for anything that comes across, and echo it back.
//      */
    int iii;
    do{
        data_length = 255;
        status = c_serial_read_data( m_port, data, &data_length, &m_lines );
        if( status < 0 ){
            break;
        }
        //   mexPrintf( "Got %d bytes of data\n", data_length );
        iii += data_length;
        for( x = 0; x < data_length; x++ ){
            mexPrintf( "%c", data[ x ]);
        }
//        mexEvalString("pause(.001);");
    }while( iii<38 );
    c_serial_close( m_port );
}
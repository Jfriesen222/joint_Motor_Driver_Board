SerialPort_BAUD_9600 = 9600;
SerialPort_DATABITS_8 = 8;
SerialPort_STOPBITS_1 = 1;
SerialPort_PARITY_NONE = 0;
SerialPort_FLOWCTRL_NONE = 0;
terminator = char(13);
commPort = ...
javax.comm.CommPortIdentifier.getPortIdentifier('com11');
serialPort = open(commPort);
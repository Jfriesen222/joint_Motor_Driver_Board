clear variables
addpath('Mex_C_Serial_Interface')
baud = 921600;
s(1) = openPort('COM11',baud);
s(2) = openPort('COM12',baud);
s(3) = openPort('COM13',baud);

motorCommands1int32 = int32([3762, 8597, 256]);
motorCommands2int32 = int32([0235, 23050, 489]);
motorCommands3int32 = int32([02035, 203050, 4890]);
message = [[uint8('$'), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint8('*'), 0 ,   uint8(13),  uint8(10)];
           [uint8('$'), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint8('*'), 0 ,   uint8(13),  uint8(10)];
           [uint8('$'), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint8('*'), 0 ,   uint8(13),  uint8(10)]];
bits = [typecast(motorCommands1int32,'uint8');
        typecast(motorCommands2int32,'uint8');
        typecast(motorCommands3int32,'uint8')];
% Fill data into our message structure
message(1,2:13) = bits(1,:); message(2,2:13) = bits(2,:); message(3,2:13) = bits(3,:);
% Compute Checksums
checksums = uint8([0; 0; 0]) ;
for count = 1:length(bits(1,:))  
    checksums = bitxor(checksums,bits(:,count));  % checksum calculation
end
% Fill checksum into message structure
message(:,15) = checksums;
for iii = 1:3
    writePort(s(iii),uint8(message(iii,:)));
end
pause(0.1)
tic
xx = zeros(1000,1);
for i = 1:1000
    % Convert the integer commands into bytes
    bits = [typecast(motorCommands1int32,'uint8');
        typecast(motorCommands2int32,'uint8');
        typecast(motorCommands3int32,'uint8')];
    % Fill data into our message structure
    message(1,2:13) = bits(1,:); message(2,2:13) = bits(2,:); message(3,2:13) = bits(3,:);
    % Compute Checksums
    checksums = uint8([0; 0; 0]) ;
    for count = 1:length(bits(1,:))       % checksum calculation ignores $ at start
        checksums = bitxor(checksums,bits(:,count));  % checksum calculation
    end
    % Fill checksum into message structure
    message(:,15) = checksums;
    %Write messages to serial ports and flush
    pauses(1/500 - toc)
    xx(i) = toc;
    %record timing
    tic
    for iii =1:3
        [dummy, bytesLeft] =  readWrap(s(iii),37);
        if(length(dummy) == 37)
            bitsIn(iii,:) = dummy;
        else
            disp('data missing'); disp(iii);
        end
    end
    %Compute checksum
    checksums = uint8([0; 0; 0]) ;
    for count = 2:33       % checksum calculation ignores $ at start
        checksums = bitxor(checksums,bitsIn(:,count));  % checksum calculation
    end
    if(checksums(1) ~= bitsIn(1,end-2))
        disp('woopsy1');
    end
    if(checksums(2) ~= bitsIn(2,end-2))
        disp('woopsy2')
    end
    if(checksums(3) ~= bitsIn(3,end-2))
        disp('woopsy3')
    end
    for iii = 1:3
        writePort(s(iii),uint8(message(iii,:)));
    end
    
    
    for ji = 1:3
        Measures(ji,:) = [typecast(bitsIn(ji,[ 2:5,  10:13,  18:21, 26:29]), 'int32')   typecast(bitsIn(ji,[6:9, 14:17,22:25, 30:33]), 'int16')];
    end
    restLengths = Measures(:, [ 1 3]);
    angles = Measures(:, [ 6 10]);
    forces = Measures(:, [ 5 9]);
    restLengths = restLengths([1 4 2 5 3 6]);
    angles = angles([1 4 2 5 3 6]);
    forces = forces([1 4 2 5 3 6]);
    %disp([restLengths forces angles])
    % g = dec2bin(15,4);
    % disp(g)
end
for i = 1:3
    closePort(s(i))
end
mean(xx(3:end))
plot(xx(3:end))
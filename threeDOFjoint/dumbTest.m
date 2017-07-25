fclose(instrfind);
s = serial('COM11','BaudRate',460800,'Timeout',1);
fopen(s);
tic
flushinput(s) 
%fprintf(s,'$12345678910');
for i = 1:10000
    xx(i) = toc;
    tic
    fprintf(s,'$12345678910');
    flushoutput(s) 
    x = fread(s,34,'uint8');
    disp( typecast(uint8(x([5:8, 12:15,19:22, 26:29 ])), 'int16')' )
    
end
mean(xx)
plot(xx)
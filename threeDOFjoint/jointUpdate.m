function jointUpdate(joint1,jointCommandPlot1,jointDynamicsPlot1,pStruct1,s1)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

%create some persistent variables for objects and structs
persistent joint jointCommandPlot pStruct i message bitsIn s
global dumb1 dumb2
if nargin>1
    tic
    joint = joint1;
    jointCommandPlot = jointCommandPlot1;
    jointDynamicsPlot = jointDynamicsPlot1;
    pStruct = pStruct1;
    getStaticTensions(joint,10);
    i = 0;
    s = s1;
    %     myData = uint8('$-223573,-223573,-223573,-223573,-223573,-223573*2C');
    %     fileID = fopen('restlengths.dat','w');
    %     fwrite(fileID,myData,'uint8');
    %     fclose(fileID);
    %     s = memmapfile('restlengths.dat',...
    %         'Format','uint8',...
    %         'Writable',true);
    
    %%%%%%%%%%%%% open serial port %%%%%%%%%%%%%%
    bitsIn = zeros(3,37);
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %myData = int32([1:10000]);
    message = [[uint8('$'), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint8('*'), 0 ,   uint8(13),  uint8(10)];
           [uint8('$'), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint8('*'), 0 ,   uint8(13),  uint8(10)];
           [uint8('$'), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint8('*'), 0 ,   uint8(13),  uint8(10)]];
    for iii = 1:3
        writePort(s(iii),uint8(message(iii,:)));
    end
end
i = i+1;
%%%%%%%%%%%%%%%%%% update Variables %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% set variables from persistent structure
r1 = pStruct.r1; h1 = pStruct.h1; r2 = pStruct.r2; h2 = pStruct.h2; NR = pStruct.NR; minQ = pStruct.minQ; tspan = pStruct.tspan;
% set variables from sliders below
T = pStruct.sliders(1).Value;

%T
%.Value; 
G = pStruct.sliders(2).Value; P = pStruct.sliders(3).Value;
Gain = pStruct.sliders(4).Value;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%% compute quaternions %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
quat = DCM2quat(getHG_Tform(T,G,P));
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%% update spine nodes in command plot and spine object %%%%%%%%%%%%%%%%%%%%%
jointNodes = get3DOFJointNodes(h1,r1,r2,h2,NR,quat);
joint.nodePoints = jointNodes;
jointCommandPlot.nodePoints = jointNodes;
tensions = getStaticTensions(joint,60);
restLengths = joint.simStruct.stringRestLengths(4:end);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%send Command%%%%%%%%%%%%%%%%%

b = [-397770.190476191,-395251.571428572,-414110.904761905,-409412.333333333,-405040.095238095,-404712.190476191;
    1014571.42857143,1022714.28571429,1016714.28571429,1034333.33333333,1018952.38095238,1012571.42857143];
restLengthsInt32 = zeros(1,6);
for ii = 1:6
    restLengthsInt32(ii) = round(b(1,ii) + b(2,ii)*restLengths(ii));
end
% Convert the integer commands into bytes
bits = [typecast(int32([restLengthsInt32([1 2]),  Gain]),'uint8');
    typecast(int32([restLengthsInt32([3 4]),  Gain]),'uint8');
    typecast(int32([restLengthsInt32([5 6]),  Gain]),'uint8')];
% Fill data into our message structure
message(1,2:13) = bits(1,:); message(2,2:13) = bits(2,:); message(3,2:13) = bits(3,:);
% Compute Checksums
checksums = uint8([0; 0; 0]) ;
for count = 1:length(bits(1,:))       % checksum calculation ignores $ at start
    checksums = bitxor(checksums,bits(:,count));  % checksum calculation
end
% Fill checksum into message structure
message(:,15) = checksums;
%pause(0.01);
%toc
updatePlot(jointCommandPlot);
%drawnow
pause(2e-3)


pauses(1/100 - toc);
dumb1 = [dumb1 toc];
dumb2 = [dumb2 T];
tic;
for iii =1:3
    [dummy, bytesLeft] =  readPort(s(iii),50);
    if(length(dummy) == 37)
        bitsIn(iii,:) = dummy;
        %disp(bitsIn(iii,:))
    else
        disp('data missing');
    end
end
bitsIn = uint8(bitsIn);
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
%disp(message)
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
measures = sprintf(' %12i',[restLengths angles forces]);
disp(measures)
%Pause to keep timing
% pauses(0.01 - toc)
% %record timing
% xx(i) = toc;
% tic

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%Compute Command and update dynamics if feasible command is generated%%%%%%%

%disp(joint.simStruct.stringRestLengths);
%dynamicsUpdate(joint,tspan);
%jointDynamicsPlot.nodePoints = joint.ySim(1:end/2,:);
%disp(jointDynamicsPlot.nodePoints)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 
% if mod(i,20) == 0
%     updatePlot(jointCommandPlot);
%     %updatePlot(jointDynamicsPlot);
%     drawnow %plot it up
% end
end


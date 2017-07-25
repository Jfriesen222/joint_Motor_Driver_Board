out = timerfind;
delete(out);
addpath('Mex_C_Serial_Interface')
addpath('Small_Quad_Prog')
clc; close all; clear all;
opengl hardware
addpath('..\tensegrityObjects')
global dumb1 dumb2; 
dumb = 0;
r1 = 0.150;  %radius bot tetrahedron
h1 = 0.172;
h2 = 0.163;%0.0436;%  %radius top tetrahedron
r2 = 0.045;%%height top
NR = 1 - 0.029/0.172; %ratio of nesting
Kp = 350;
%Equilibrium vals
% -173207      -168644      -168475      -167249      -192291      -188374          574          558          545          576          565          567            8           10            9            8            7           10
quat = DCM2quat(getHG_Tform(0,0,0));
jointNodes = get3DOFJointNodes(h1,r1,r2,h2,NR,quat);
T= 0 ; G = 0; P = 0;

stringStiffness = [50000 * ones(3,1); 1000*ones(6,1)];
barStiffness = 50000*ones(13,1);
stringDamping = 5000*ones(9,1);
barDamping = 5000*ones(13,1);
nodalMass = 0.1*ones(8,1);
nodalMass(end) = 0.3;
nodalMass(1:3) = 0;
g=9.81;
F = [zeros(8,1), zeros(8,1), -nodalMass*9.81];
F(1:3,3) = -sum(F(:,3))*ones(3,1)/3;

tspan = 0.025;

delT = 0.003;
minQ = 0;

lims = 0.25;

bars = [1 2 3 4 4 4 5 6 7 8 8 8 8;
    2 3 1 5 6 7 6 7 5 5 6 7 4];

strings = [1 2 3 1 1 3 3 2 2;
    4 4 4 5 7 7 6 6 5];


%%%%%%%%%%%%%% Create objects for spine IK/dynamics, and plotting objects %%%%%%
% Pass all variables to the TensegrityStructure constructor to create the
% object which we call spine. This object contains methods for inverse
% kinematics as well as dynamics
joint = TensegrityStructure(jointNodes, strings, bars, F,stringStiffness,...
    barStiffness,stringDamping,nodalMass,delT,[],[]);

theta = (0:(2*pi/24):2*pi)';
ringNodes = [r1*sin(theta), r1*cos(theta), jointNodes(1,3)*ones(size(theta)) ];
ringStrings = [1;3];
ringBars = [1:length(theta);
    length(theta), 1:length(theta)-1];

jointCommandPlot = TensegrityPlot(jointNodes, strings, bars(:,4:end), 0.005,0.001);
%jointDynamicsPlot = TensegrityPlot(jointNodes, strings, bars(:,4:end), 0.005,0.001);
ringPlot = TensegrityPlot(ringNodes, ringStrings, ringBars, 0.005,0.001);

f = figure('units','normalized','outerposition',[0.25 0.25 0.4 0.65]);

%%%%%%%% IK Subplot %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ax = axes; % subplot(1,2,1,'Parent',f,'units','normalized','outerposition',[0.01 0.1 0.48 0.9]);
hold on
% use a method within TensegrityPlot class to generate a plot of the
% structure
generatePlot(jointCommandPlot,ax)
updatePlot(jointCommandPlot)

generatePlot(ringPlot,ax)
updatePlot(ringPlot)
%settings to make it pretty
axis equal
view(3)
grid on
light('Position',[0 0 1],'Style','local')
%lighting flat
lighting gouraud
colormap cool% winter
xlim([-lims lims])
ylim([-lims lims])
zlim([0 1.4*lims])
title('Static IK Command');

%%%%%% Dynamics Subplot %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ax = subplot(1,2,2,'Parent',f,'units','normalized','outerposition',...
%     [0.51 0.1 0.48 0.9]);
% 
% % use a method within TensegrityPlot class to generate a plot of the
% % structure
% generatePlot(jointDynamicsPlot,ax)
% updatePlot(jointDynamicsPlot)
% 
% generatePlot(ringPlot,ax)
% updatePlot(ringPlot)
% 
% axis equal
% view(3)
% grid on
% light('Position',[0 0 1],'Style','local')
% xlim([-lims lims])
% ylim([-lims lims])
% zlim([-0.01 1.6*lims])
% title('Dynamic Response');
% Create an object calls from the class TensegrityCallbackFunctions which is
% just a function wrapper to hold a vector of persistent values, and some update functions
% It also holds a function for creating sliders
%set(ax,'DrawMode','fll
% ax.GridAlphaMode = 'manual'; 
% ax.ZTickLabelMode = 'manual';
% ax.ZTickMode = 'manual';
% ax.YTickMode = 'manual';
% ax.YTickLabelMode = 'manual';
% ax.XTickLabelMode = 'manual';
% ax.XTickMode = 'manual';
% ax.TickDirMode = 'manual';
% ax.MinorGridColorMode = 'manual';
% ax.GridColorMode = 'manual';
% ax.GridAlphaMode = 'manual';
% ax.CameraUpVectorMode = 'manual';
% ax.CameraPositionMode = 'manual';
% ax.CLimMode = 'manual';
% ax.ALimMode = 'manual';
% ax.CameraUpVectorMode = 'manual';
% ax.CameraUpVectorMode = 'manual';
% ax.CameraUpVectorMode = 'manual';

calls = TensegrityCallbackFunctions([T, G ,P, Kp]);

%Use the function to create some slider below and attach some callback
%functions to update angle, bed axis, twist, and NR
bendLims = 1.1;
sliders(1) = calls.makeSlider(f,0,0,[-bendLims bendLims],'Theta');
sliders(2) = calls.makeSlider(f,475,0,[-bendLims bendLims],'Gamma');
sliders(3) = calls.makeSlider(f,0,100,[-0.8 0.8],'Phi');
sliders(4) = calls.makeSlider(f,475,100,[20 500],'K_p');


pStruct = struct('minQ',minQ,'tspan',tspan,'r1',r1,'h1',h1,'r2',r2,'h2',h2,'NR',NR,'sliders',sliders);
vec1 = [T,G,P,Kp];

baud = 921600;
s(1) = openPort('COM11',baud);
s(2) = openPort('COM12',baud);
s(3) = openPort('COM13',baud);
jointDynamicsPlot = 0;
jointUpdate(joint, jointCommandPlot,jointDynamicsPlot,pStruct,s);

%Create a function handle which only passes the vector of values we will be
%updating
%Create a timer to update everything, 20 fps should
%look smooth prob best not to go below this
for i = 1:100000
    timerUpdate(calls,@jointUpdate)
end
for i = 1:3
    closePort(s(i))
end
% for i = 1:3
%     closePort(s(i))
% end
% t = timer;
% t.TimerFcn = @(myTimerObj, thisEvent) timerUpdate(calls,jointUpdates);
% t.Period = tspan;
% t.ExecutionMode = 'fixedRate';
% start(t);





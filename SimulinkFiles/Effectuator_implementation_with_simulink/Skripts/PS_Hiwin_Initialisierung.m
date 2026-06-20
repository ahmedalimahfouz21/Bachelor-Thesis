%% Laden der Bibliothek
global AxisLib;
AxisLib.dllPath = 'C:\Users\devekos\Documents\DEVEKOS-Demonstrator\HWin_DLL\build_x64_VS2017_newPc\Debug\HwinAxisC.dll';
AxisLib.headerPath = 'C:\Users\devekos\Documents\DEVEKOS-Demonstrator\HWin_DLL\HwinAxisC.hpp';
AxisLib.Name = 'HwinAxisC';
loadlibrary(AxisLib.dllPath, AxisLib.headerPath);

%% Parameter RT
global vel;
vel = 200;
global acc
acc = 200;
global dcc;
dcc = 200;
global decKill;
decKill = 200;
global smoothTime;
smoothTime = 10;
global inPosWidth;
inPosWidth = 2;
global posLimitPos;
posLimitPos = 360000;
global negLimitPos;
negLimitPos = -360000;
global waitTimeHoming;
waitTimeHoming = 200000;
global waitTimeReset;
waitTimeReset = 10000;
global velJog;
velJog = 50;

%% Initialisierung und Homing der Achse
rtPtr = 'X_';
global c2Ptr;
c2Ptr = calllib(AxisLib.Name,'w_MPI_CreateAxis',rtPtr,0,2);
calllib(AxisLib.Name,'w_MPI_Disconnect');
%input('Disconnect done');
calllib(AxisLib.Name,'w_MPI_ConnectMegaulink',0);
%input('Connect done');
calllib(AxisLib.Name,'w_MPI_ResetDrive',c2Ptr);
calllib(AxisLib.Name,'w_MPI_WaitServoReady',c2Ptr,waitTimeReset);
calllib(AxisLib.Name,'w_MPI_InitialAxis',c2Ptr,vel,acc,dcc,decKill,smoothTime,inPosWidth,posLimitPos,-negLimitPos);
calllib(AxisLib.Name,'w_MPI_StartHome',c2Ptr);
calllib(AxisLib.Name,'w_MPI_WaitHomeOver',c2Ptr,waitTimeHoming);
% pause(25);
% calllib(AxisLib.Name,'w_MPI_MoveAbsolute',c2Ptr,5,100,0,0);
% pause(1);
% calllib(AxisLib.Name,'w_MPI_SetZero',c2Ptr);
cIsHomedPtr = libpointer('int32Ptr',0);
calllib(AxisLib.Name,'w_MPI_IsHomed',c2Ptr,cIsHomedPtr);
cIsHomedPtr = get(cIsHomedPtr);
if cIsHomedPtr.Value
    disp('RT is homed.');
else
    disp('Error Homing');
end
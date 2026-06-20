%%
NET.addAssembly('LinUDP');

global axX axY axZ axC;
global axX_maxAcc axX_maxDec axY_maxAcc axY_maxDec axZ_maxAcc axZ_maxDec axC_maxAcc axC_maxDec;

axisX='192.168.1.86';
axisY='192.168.1.87';
axisZ='192.168.1.88';
axisC='192.168.1.89';

axX = axisX;
axY = axisY;
axZ = axisZ;
axC = axisC;

axX_maxAcc = 1.5;
axX_maxDec = 1.5;
axY_maxAcc = 1.5;
axY_maxDec = 1.5;
axZ_maxAcc = 1.5;
axZ_maxDec = 1.5;
axC_maxAcc = 0.3;
axC_maxDec = 0.3;

% axX_maxAcc = 1;
% axX_maxDec = 1;
% axY_maxAcc = 1;
% axY_maxDec = 1;
% axZ_maxAcc = 1;
% axZ_maxDec = 1;
% axC_maxAcc = 0.3;
% axC_maxDec = 0.3;

TargetPort='49360';

ACI=LinUDP.ACI;
ACI.CreateTargetAddressList;
ACI.ClearTargetAddressList %brauch man eventuell nicht
ACI.SetTargetAddressList(axisX, TargetPort);
ACI.SetTargetAddressList(axisY, TargetPort);
ACI.SetTargetAddressList(axisZ, TargetPort);
ACI.SetTargetAddressList(axisC, TargetPort);
ACI.ActivateConnection('192.168.1.90','41136');

ACI.setSwitchOnBit(axisX,false);
ACI.setSwitchOnBit(axisY,false);
ACI.setSwitchOnBit(axisZ,false);
% ACI.setSwitchOnBit(axisC,false);

pause(2);

%%
if(~ACI.isSwitchOnActive(axX))
    ACI.SwitchOn(axX);
    disp('Axis X switch on')
end
if(~ACI.isSwitchOnActive(axY))
    ACI.SwitchOn(axY);
    disp('Axis Y switch on')
end
if(~ACI.isSwitchOnActive(axZ))
    ACI.SwitchOn(axZ);
    disp('Axis Z switch on')
end
% if(~ACI.isSwitchOnActive(axC))
%     ACI.SwitchOn(axC);
%     disp('Axis C switch on')
% end

pause(2);

%%
if(ACI.isSwitchOnActive(axX))
    ACI.Homing(axX)
    disp('Axis X homing')
end
if(ACI.isSwitchOnActive(axY))
    ACI.Homing(axY)
    disp('Axis Y homing')
end
if(ACI.isSwitchOnActive(axZ))
    ACI.Homing(axZ)
    disp('Axis Z homing')
end
% if(ACI.isSwitchOnActive(axC))
%     ACI.Homing(axC)
%     disp('Axis C homing')
% end

pause(2);
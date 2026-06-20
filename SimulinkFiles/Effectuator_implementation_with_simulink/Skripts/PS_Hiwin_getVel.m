function [cVelValue] = PS_Hiwin_getVel()

global AxisLib;
global c2Ptr;
cVelPtr = libpointer('doublePtr',20);
calllib(AxisLib.Name,'w_MPI_GetFeedbackVel',c2Ptr,cVelPtr);
cVel = get(cVelPtr);
cVelValue = cVel.Value;
end


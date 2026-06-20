function [cPosValue] = PS_Hiwin_getPos()

global AxisLib;
global c2Ptr;
cPosPtr = libpointer('doublePtr',20);
calllib(AxisLib.Name,'w_MPI_GetFeedbackPos',c2Ptr,cPosPtr);
cPos = get(cPosPtr);
cPosValue = cPos.Value;
end


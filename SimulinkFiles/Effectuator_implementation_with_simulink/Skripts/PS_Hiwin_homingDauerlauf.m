function PS_Hiwin_homingDauerlauf()

global AxisLib;
global c2Ptr;
cPosPtr = libpointer('doublePtr',20);
calllib(AxisLib.Name,'w_MPI_GetFeedbackPos',c2Ptr,cPosPtr);
cPos = get(cPosPtr);
if cPos.Value > 180
    targetPos = 360;
else
    targetPos = 0;
end
calllib(AxisLib.Name,'w_MPI_MoveAbsolute',c2Ptr,targetPos,100,0,0);
pause(3);
calllib(AxisLib.Name,'w_MPI_SetZero',c2Ptr);
end


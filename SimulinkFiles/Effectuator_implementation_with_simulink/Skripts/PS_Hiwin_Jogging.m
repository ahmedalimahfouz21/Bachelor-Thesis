function PS_Hiwin_Jogging(BoolDirection)

global AxisLib;
global c2Ptr;
global velJog;
cIsMovingPtr = libpointer('int32Ptr',0);
calllib(AxisLib.Name,'w_MPI_IsMoving',c2Ptr,cIsMovingPtr);
cIsMovingPtr = get(cIsMovingPtr);
if cIsMovingPtr.Value
    PS_Hiwin_STOP();
elseif BoolDirection
    calllib(AxisLib.Name,'w_MPI_JogPositive',c2Ptr,velJog);
else
    calllib(AxisLib.Name,'w_MPI_JogNegative',c2Ptr,velJog);
end


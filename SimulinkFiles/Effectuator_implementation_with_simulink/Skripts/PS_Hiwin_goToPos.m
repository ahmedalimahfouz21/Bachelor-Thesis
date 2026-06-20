function PS_Hiwin_goToPos(targetPos,vel)

global AxisLib;
global c2Ptr;
calllib(AxisLib.Name,'w_MPI_MoveAbsolute',c2Ptr,targetPos,vel,0,0);
end


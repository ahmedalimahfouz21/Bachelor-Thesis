function PS_Hiwin_moveRel(deltaPos,vel)

global AxisLib;
global c2Ptr;
calllib(AxisLib.Name,'w_MPI_MoveRelative',c2Ptr,deltaPos,vel,0,0);
end


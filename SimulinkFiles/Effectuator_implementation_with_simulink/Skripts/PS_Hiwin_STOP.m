function PS_Hiwin_STOP()

global AxisLib;
global c2Ptr;
calllib(AxisLib.Name,'w_MPI_StopMotion',c2Ptr);
end


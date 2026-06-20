function []  = V_2_x_STOP(axis)

NET.addAssembly('LinUDP');

global axX axY axZ axC;
global axX_maxDec axY_maxDec axZ_maxDec axC_maxDec;

obj=LinUDP.ACI;

switch axis
    case 1
        obj.LMmt_Stop(axX,axX_maxDec);
    case 2
        obj.LMmt_Stop(axY,axY_maxDec);
    case 3
        obj.LMmt_Stop(axZ,axZ_maxDec);
    case 4
        obj.LMmt_Stop(axC,axC_maxDec);
end

end
function []  = V_2_x_moveRel(axis,deltaS,vel)

NET.addAssembly('LinUDP');

global axX axY axZ axC;
global axX_maxAcc axX_maxDec axY_maxAcc axY_maxDec axZ_maxAcc axZ_maxDec axC_maxAcc axC_maxDec;

obj=LinUDP.ACI;

switch axis
    case 1
        obj.LMmt_MoveRel(axX,deltaS,vel,axX_maxAcc,axX_maxDec);
    case 2
        obj.LMmt_MoveRel(axY,deltaS,vel,axY_maxAcc,axY_maxDec);
    case 3
        obj.LMmt_MoveRel(axZ,deltaS,vel,axZ_maxAcc,axZ_maxDec);
    case 4
        obj.LMmt_MoveRel(axC,deltaS,vel,axC_maxAcc,axC_maxDec);
end

end
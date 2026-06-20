function []  = V_2_x_goToPos(axis,targetPos,vel)

NET.addAssembly('LinUDP');

global axX axY axZ axC;
global axX_maxAcc axX_maxDec axY_maxAcc axY_maxDec axZ_maxAcc axZ_maxDec axC_maxAcc axC_maxDec;

obj=LinUDP.ACI;

switch axis
    case 1
        obj.LMmt_MoveAbs(axX,targetPos,vel,axX_maxAcc,axX_maxDec);
    case 2
        obj.LMmt_MoveAbs(axY,targetPos,vel,axY_maxAcc,axY_maxDec);
    case 3
        obj.LMmt_MoveAbs(axZ,targetPos,vel,axZ_maxAcc,axZ_maxDec);
    case 4
        obj.LMmt_MoveAbs(axC,targetPos,vel,axC_maxAcc,axC_maxDec);
end

end
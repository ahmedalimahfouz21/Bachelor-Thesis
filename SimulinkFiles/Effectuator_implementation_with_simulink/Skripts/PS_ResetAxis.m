function PS_ResetAxis(axis)
NET.addAssembly('LinUDP');

ACI = LinUDP.ACI;

global axX axY axZ;

switch axis
    case 1
        ACI.setSwitchOnBit(axX,true);
        ACI.setBit6(axX,false);
        ACI.setJogPlus(axX,false);
        ACI.setJogMinus(axX,false);
        ACI.setBit10(axX,false);
        ACI.setHomingBit(axX,false);
        ACI.setBit12(axX,false);
        ACI.setBit13(axX,false);
        ACI.setBit14(axX,false);
        ACI.setBit15(axX,false);
    case 2
        ACI.setSwitchOnBit(axY,true);
        ACI.setBit6(axY,false);
        ACI.setJogPlus(axY,false);
        ACI.setJogMinus(axY,false);
        ACI.setBit10(axY,false);
        ACI.setHomingBit(axY,false);
        ACI.setBit12(axY,false);
        ACI.setBit13(axY,false);
        ACI.setBit14(axY,false);
        ACI.setBit15(axY,false);
    case 3
        ACI.setSwitchOnBit(axZ,true);
        ACI.setBit6(axZ,false);
        ACI.setJogPlus(axZ,false);
        ACI.setJogMinus(axZ,false);
        ACI.setBit10(axZ,false);
        ACI.setHomingBit(axZ,false);
        ACI.setBit12(axZ,false);
        ACI.setBit13(axZ,false);
        ACI.setBit14(axZ,false);
        ACI.setBit15(axZ,false);
end
end


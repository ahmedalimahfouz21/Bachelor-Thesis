function []  = V_2_x_Jogging(axis)

NET.addAssembly('LinUDP');

global axX axY axZ axC;

obj=LinUDP.ACI;

switch axis
    case 1
        obj.JogPlus(axX);
    case 2
        obj.JogPlus(axY);
    case 3
        obj.JogPlus(axZ);
    case 4
        obj.JogPlus(axC);
end

end
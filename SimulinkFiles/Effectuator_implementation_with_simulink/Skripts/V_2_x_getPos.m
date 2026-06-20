function [pos,timeStamp]  = V_2_x_getPos(axis)
NET.addAssembly('LinUDP');

global axX axY axZ axC;

obj=LinUDP.ACI;

switch axis
    case 1
        pos = obj.getActualPosWithTimestamp(axX).value;
        timeStamp = double(obj.getActualPosWithTimestamp(axX).Timestamp);
    case 2
        pos = obj.getActualPosWithTimestamp(axY).value;
        timeStamp = double(obj.getActualPosWithTimestamp(axY).Timestamp);
    case 3
        pos = obj.getActualPosWithTimestamp(axZ).value;
        timeStamp = double(obj.getActualPosWithTimestamp(axZ).Timestamp);
    case 4
        pos = obj.getActualPosWithTimestamp(axC).value;
        timeStamp = double(obj.getActualPosWithTimestamp(axC).Timestamp);
end

%timeStamp = 0;

end
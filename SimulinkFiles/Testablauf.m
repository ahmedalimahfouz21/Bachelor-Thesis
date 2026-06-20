
%% Bahnkurve

setPoints(1,:) = [230,190,110,70,110,190,linspace(230,725,10),230];
setPoints(2,:) = [75,5,5,75,145,145,75,linspace(75,75,10)];
setPoints(3,:) = [4,4,0,0,0,4,4,3,2,1,0,0,1,2,3,4,4];

% Koordinationsmatrix

kooVec(1,:) = [0,0,0,0,0,0,0,8,11,11,11,11,0,0,0,0];
kooVec(2,:) = [7,7,7,7,3,3,3,3,7,7,7,7,3,3,3,3];
kooVec(3,:) = [7,7,7,7,3,3,3,3,12,12,7,7,3,3,3,3];
kooVec(4,:) = [8,88,8,7,3,3,3,3,7,7,7,7,3,3,3,3];
kooVec(5,:) = [7,7,7,7,3,3,3,3,7,7,7,7,3,3,6,6];

%% UDP Send Bahnkurve get Koordinationsvektor

[dataDimZ,dataDimS] = size(setPoints);
output = [1;dataDimZ;dataDimS;setPoints(:)];
s = udp('192.168.5.100');
s.LocalHost = '192.168.5.90';
s.RemotePort = 8080;
s.ByteOrder = 'littleEndian';
fopen(s);
fwrite(s,output,'int32');
fclose(s);

u = udp('192.168.5.100');
u.LocalHost = '192.168.5.90';
u.LocalPort = 8000;
u.ByteOrder = 'littleEndian';
u.Timeout = 10;
fopen(u);
input = fread(u,4,'int32')
fclose(u);
massageReceived = input(1)
dataDim =[input(2),input(3)];
dataReceived = input(4:end);
koordinationsvektor = reshape(dataReceived,dataDim);

% UDP Send Koordinationsmatrix get Achse bereit
[dataDimZ,dataDimS] = size(kooVec);
output2 = [2;dataDimZ;dataDimS;kooVec(:)];
v = udp('192.168.5.100');
v.LocalHost = '192.168.5.90';
v.RemotePort = 8080;
v.ByteOrder = 'littleEndian';
fopen(v);
fwrite(v,output2,'int32');
fclose(v);

w = udp('192.168.5.100');
w.LocalHost = '192.168.5.90';
w.LocalPort = 8000;
w.ByteOrder = 'littleEndian';
w.Timeout = 10;
fopen(w);
input2 = fread(w,4,'int32')
fclose(w);
massageReceived2 = input2(1);
dataDim2 =[input2(2),input2(3)];

% UDP Send Achse start get Achse fertig
output3 = [3;0;0];
x = udp('192.168.5.100');
x.LocalHost = '192.168.5.90';
x.RemotePort = 8080;
x.ByteOrder = 'littleEndian';
fopen(x);
fwrite(x,output3,'int32');
fclose(x);

pause(0.3);

y = udp('192.168.5.100');
y.LocalHost = '192.168.5.90';
y.LocalPort = 8000;
y.ByteOrder = 'littleEndian';
%y.Timeout = 120;
fopen(y);
input3 = fread(y,4,'int32')
fclose(y);
%massageReceived3 = input3(1);
%dataDim3 =[input3(2),input3(3)];

u=udp('192.168.5.100');
u.LocalHost='192.168.5.90';
u.LocalPort=8000;
u.RemotePort=8000;
u.InputDatagramPacketSize=2408;
u.InputBufferSize=2408;
u.EnablePortSharing='on';
u.ByteOrder = 'littleEndian';
t=1;
while (1)
t=t+1;
fopen(u);
%input=fread(u,2048,'int32');
%(input)'
output=[t;2*t;3*t;4*t];
fwrite(u,output,'int32');
fclose(u);
%pause(.1);
end
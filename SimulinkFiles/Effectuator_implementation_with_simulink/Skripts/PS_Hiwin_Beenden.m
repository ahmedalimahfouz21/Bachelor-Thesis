% Beenden der Verbindung
global AxisLib;
global c2Ptr;
calllib(AxisLib.Name,'w_MPI_Disconnect');
calllib(AxisLib.Name,'w_MPI_DestroyAxis',c2Ptr);

%% Auswerfen der Bibliothek
%unloadlibrary(AxisLib.Name);
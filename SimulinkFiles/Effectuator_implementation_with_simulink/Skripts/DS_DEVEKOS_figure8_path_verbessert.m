%% Verwende vordefinierte Setpoints (Kreis mit Z-Komponente)
close all
% Config
numCircles = 4;

%Versuch symmetrische Setpunkte zu setzen
pointsPerCircle = 80;


% r = 50;
% m_x = r;
% m_y = r;
z_dip = 65; % 70 ist max
% reale Messwerte im Achsraum
r = 62; %Update am 18. Nov
m_x = 77.50; %74;
m_y = 62; %62;


% //
s = linspace(0,numCircles*2*pi,numCircles*pointsPerCircle);

setPoints_x = -cos(s) .* r + m_x;
setPoints_y = -sin(s) .* r + m_y; % Achtung hier habe ich wieder auf "im Uhrzeigersinn" gedreht

% Umkehrpunkte bei Nullstellen des y-Vektors
idxFlip = find(((setPoints_y(1:end-1)-m_y) .*...
    (setPoints_y(2:end)-m_y)) < 0);


for ii=1:length(idxFlip)-1

    switch ii
        case 1
            setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)) = ...
               2*m_x + 2*r-(setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)));% r*2 - (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)));
        case 2
            setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)) = ...
              4*r + (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1))); %r*3 + (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1))); % war m_x + r*3
        case 3
            setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)) = ...
              4*r + (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1))); % r*3 + (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1))); % war m_x + r*3
        case 4
            setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)) = ...
                2*m_x + 2*r -(setPoints_x(idxFlip(ii)+1:idxFlip(ii+1))); %r*2 - (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)));
        otherwise
           setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)) = ...
                (setPoints_x(idxFlip(ii)+1:idxFlip(ii+1)));
    end

end


z = zeros(1,size(setPoints_x,2));
% Dip1
dip_length=5;
z_x = -dip_length:dip_length; % runter und direkt wieder hoch
z_y = z_dip.*(-z_x.^2 + dip_length^2)./(dip_length^2);
z_idx = idxFlip(3); % war 90

z(z_idx:z_idx+length(z_x)-1) = z_y;

% Dip2 + Flyby
dip_length=50;
z_idx2 = idxFlip(5)+10; % war 210
z_x = -dip_length:dip_length; % Wenn -dip:0 dann nur runter, wenn 0:dip dann hoch - daran denken dann untern hold zu machen
z_y = z_dip.*(-z_x.^2 + dip_length^2)./(dip_length^2); %TODO: Hier Normierung, damit Dip max bis 35 geht

z(z_idx2:z_idx2+length(z_x)-1) = z_y;
% z = [z_y z];
z(1) = z(end) + 3;
setPoints_z = z;




setPoints_x = setPoints_x ./ 1000;
setPoints_y = setPoints_y ./ 1000;
setPoints_z = setPoints_z ./ 1000;

figure(1)
plot(setPoints_x,setPoints_y);
hold on 
figure(2)
plot3(setPoints_x(1:end-1),setPoints_y(1:end-1),setPoints_z(1:end-1));
hold on

ax = gca;
ax.YDir = 'reverse';
ax.ZDir = 'reverse';

% setPoints_z = zeros(1,size(setPoints_x,2));

setP_mat = [setPoints_x;setPoints_y;setPoints_z];
%Dauerlauftest zehn Durchgänge
% setP_mat = repmat(setP_mat, 1, 10);
setP_mat = setP_mat(:,2:end);
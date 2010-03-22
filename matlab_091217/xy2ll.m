function [long,lat] = xy2ll(pose)  

r_earth = 6371000.785;          % [m] earth radius

%long    = pose(1)*180/(pi*r_earth*cos(-pose(2)/r_earth));
%lat     = -pose(2)*180/(pi*r_earth);

long    = pose(2,1)*180/(pi*r_earth*cos(pose(1,1)/r_earth));   % -pose(2,1)
lat     = pose(1,1)*180/(pi*r_earth);

end


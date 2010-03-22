function [pose] = ll2xy(long, lat)

r_earth     = 6371000.785;                              % [m] earth radius

pose(2,1)   = r_earth*cos(lat*pi/180)*long*pi/180;     %[m]   -long
pose(1,1)   = r_earth*(lat)*pi/180;

end
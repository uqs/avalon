% creat waypoints and give the long lat for each waypoint
clear all
close all
clc


n       = 4; % numbers of waypoints, including start point
[x,y]   = ginput(n);

distNorm    = 800;

dist1       = sqrt((x(2)-x(1))^2+(y(2)-y(1))^2);

pose(1,1)   = 5.2151e+06;
pose(2,1)   = 5.16560e+05;

for i = 2:1:n;
pose(1,i) = pose(1,i-1) + (x(i)-x(i-1))/dist1*800;
pose(2,i) = pose(2,i-1) + (y(i)-y(i-1))/dist1*800;
end
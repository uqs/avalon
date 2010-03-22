% generate waypoints with the mouse cursor

lat_p       = [];
long_p      = [];
r_earth 	= 6371000.785;          % [m] earth radius

distNorm    = str2double(get(handles.dist_Norm,  'String'));
n           = str2double(get(handles.n_waypoints,'String'));

lat(1)      = str2double(get(handles.start_lat,  'String'));
long(1)     = str2double(get(handles.start_long,  'String'));
lat_p       = [lat_p lat(1)];
long_p      = [long_p long(1)];

[pose]      = ll2xy(long(1), lat(1));

[x,y] = ginput(n);
axis(handles.AxesWaypointGeneration,   'equal');
xlabel(handles.AxesWaypointGeneration, 'long [m]');
ylabel(handles.AxesWaypointGeneration, 'lat [m]');
plot(handles.AxesWaypointGeneration,   x,y);
axis(handles.AxesWaypointGeneration,   'equal');

dist_first = sqrt(( x(2)-x(1) )^2 + ( y(2)-y(1) )^2);
ratio      = distNorm/dist_first;

gridsize   = distNorm/100;
set(handles.gridSize,'String',num2str(gridsize));

for i = 2:1:n
    pose(1,i)   = (pose(1,i-1)+(y(i)-y(i-1))*ratio);
    pose(2,i)   = (pose(2,i-1)+(x(i)-x(i-1))*ratio);
    
    long(i)     = pose(2,i)*180/(pi*r_earth*cos(pose(1,i)/r_earth));   
    lat(i)      = pose(1,i)*180/(pi*r_earth);
    
    long_p      = [long_p long(i)];
    lat_p       = [lat_p lat(i)];
end

dat = [long_p ;lat_p];
columnname =   {'Long [°]', 'Lat [°]',};
handle=uitable('Data', dat', 'ColumnName', columnname);



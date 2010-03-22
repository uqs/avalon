function [pose, Z] = llh2xyz(imu_pose) % Convert lat, long, height in WGS84 to ECEF X,Y,Z 


% lat and long given in decimal degrees. 
% altitude should be given in meters

long = imu_pose(1);
lat  = imu_pose(2);
att  = imu_pose(3);
yaw  = imu_pose(4);

lat  = lat/180*pi;          %converting to radians 
long = long/180*pi;         %converting to radians 

a    = 6378137.0;           % earth semimajor axis in meters 
f    = 1/298.257223563;     % reciprocal flattening 
e2   = 2*f -f^2;            % eccentricity squared
chi  = sqrt(1-e2*(sin(lat)).^2); 

X = (a./chi + att).*cos(lat).*cos(long); 
Y = (a./chi + att).*cos(lat).*sin(long); 
Z = (a*(1-e2)./chi + att).*sin(lat); 

pose(1) = X;
pose(2) = Y;
pose(3) = yaw;

% This function generates a random IMU signal 
% --------------------------------------------


function [t_imu, imu_pose, imu_speed, imu_attitude, imu_velocity, imu_acceleration, imu_gyro, imu_temperature] = ImuSignal(t_imu, imu_pose, imu_attitude, imu_velocity, imu_acceleration, imu_gyro, imu_temperature)

    update_imu = 1/4;   % [s]
    sigma_imu = 1;      % [m] accuracy imu ? ? ?

    if t_imu >= update_imu
                
        imu_length = random('norm',0,sigma_imu);
        imu_orient = random('unif',0,2*pi);
        
        t_imu = 0;
    end

%     if t_compass >= update_compass
%         compass_orient = random('norm',0,sigma_compass);
%         t_compass = 0;
%     end
%   sens_point = pose + [gps_length*cos(gps_orient); gps_length*sin(gps_orient); compass_orient];

    imu_pose(1) = pose(1) + imu_length*cos(imu_orient);
    imu_pose(2) = pose(2) + imu_length*sin(imu_orient); 
    
    imu_speed = sqrt(imu_velocity(1)^2 + imu_velocity(2)^2);    % convert to knots?
    
    imu_attitude(3) = imu_orient;

end
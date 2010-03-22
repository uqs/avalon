
clear all
close all







A_sail      = 8.4;
dens_air    = 1.23;
V_wind      = 8;

d_wind_r = deg2rad(90);

for aoa = deg2rad(-170):deg2rad(1):deg2rad(170)
    
    d2aoa = reminderRad((-d_wind_r+aoa));
    if abs(d2aoa) <= 2*pi/180             % Hysteresis around "dead into the wind"
        c_sail_lift = 0;
    elseif abs(d2aoa) <= 25*pi/180        % 25deg = max c_sail_lift
        c_sail_lift = (2.24*abs(d2aoa)-2.24*2*pi/180);  % sign(aoa)*
        
    elseif abs(d2aoa) > 25*pi/180 && abs(d2aoa) <= 90*pi/180
        c_sail_lift = -0.79*abs(d2aoa)+1.1;           %sign(aoa)*(-0.79*abs(aoa)+0.9)
    elseif  abs(d2aoa) > 90*pi/180 && abs(d2aoa) <= pi
        c_sail_lift = 0;
    end
    
    c_sail_drag = 1.28*sin(abs(-d_wind_r+aoa));
    
    F_lift_V_w = 1/2*dens_air*c_sail_lift*V_wind^2*A_sail*cos(-d_wind_r+aoa);%*sign(-d_wind-aoa);
    F_drag_V_w = 1/2*dens_air*c_sail_drag*V_wind^2*A_sail*sin(-d_wind_r+aoa)*sign(-d_wind_r+aoa);
    
    X_sail = F_lift_V_w*sin(abs(d_wind_r)) - F_drag_V_w*cos(d_wind_r);
    if d_wind_r >= 0
        vorzeichen = 1;
    else
        vorzeichen = -1;
    end
    Y_sail = (F_lift_V_w*cos(d_wind_r) + F_drag_V_w*sin(abs(d_wind_r)))*sign(-vorzeichen);
    %N_sail = 0;
    N_sail = X_sail*0.2*sign(aoa) + Y_sail*0.4;
    
    subplot(2,1,1)
    plot(rad2deg(aoa), F_lift_V_w, '+r', rad2deg(aoa), F_drag_V_w, 'o')
    hold on
    subplot(2,1,2)
    plot(rad2deg(aoa), X_sail, 'r', rad2deg(aoa), Y_sail,'b', rad2deg(aoa), N_sail, 'g')
    hold on
end
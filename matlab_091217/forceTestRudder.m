clear all
clc
close all

dens_water = 1025;
A_rudder   = 0.085;
v_r_tot    = 1;

d_water    = deg2rad(-73);

for alpha_rudder = deg2rad(-45):deg2rad(0.1):deg2rad(45)
    
    alpha_rudder_l = alpha_rudder;
    incid_angle = -d_water + alpha_rudder;
    incid_angle = reminderRad(incid_angle);
    
    if abs(incid_angle) <= 2*pi/180            % Hysteresis around "dead into the wind"
        c_rudder_lift = 0;
    elseif abs(incid_angle) <= 25*pi/180       % 25deg = max c_sail_lift
        c_rudder_lift = (2.24*abs(incid_angle)-2.24*2*pi/180);   % sign(alpha_rudder)*
    elseif abs(incid_angle) > 25*pi/180 && abs(incid_angle) < 90*pi/180
        c_rudder_lift = -0.79*abs(incid_angle)+1.1; % sign(alpha_rudder)*(-0.79*abs(alpha_rudder)+0.9)
    elseif abs(incid_angle) >= 90*pi/180 && abs(incid_angle) <= pi
        c_rudder_lift = 0;
    end
    c_rudder_drag = 1.28*sin(abs(incid_angle));
   
    if incid_angle >= 0
        vorzeichenR = 1;
    else
        vorzeichenR = -1;
    end
    
    F_lift_v_right = 1/2*dens_water*c_rudder_lift*v_r_tot^2*A_rudder*cos(-d_water+alpha_rudder);
    F_drag_v_right = 1/2*dens_water*c_rudder_drag*v_r_tot^2*A_rudder*sin(-d_water+alpha_rudder)*sign(vorzeichenR);
    F_lift_v_left = 1/2*dens_water*c_rudder_lift*v_r_tot^2*A_rudder*cos(-d_water+alpha_rudder_l);
    F_drag_v_left = 1/2*dens_water*c_rudder_drag*v_r_tot^2*A_rudder*sin(-d_water+alpha_rudder_l)*sign(vorzeichenR);
    
    if d_water >= 0
        vorzeichenRR = 1;
    else
        vorzeichenRR = -1;
    end
    X_rudder_right = F_lift_v_right*sin(abs(d_water)) - F_drag_v_right*cos(d_water);
    Y_rudder_right = F_lift_v_right*cos(d_water)*sign(-vorzeichenR) + F_drag_v_right*sin(abs(d_water))*sign(-vorzeichenR);
    X_rudder_left = F_lift_v_left*sin(abs(d_water)) - F_drag_v_left*cos(d_water);
    Y_rudder_left = F_lift_v_left*cos(d_water)*sign(-vorzeichenR) + F_drag_v_left*sin(abs(d_water))*sign(-vorzeichenR);
    
    X_rudder = X_rudder_left + X_rudder_right;
    Y_rudder = Y_rudder_left + Y_rudder_right;
    N_rudder = Y_rudder*1.4;
    
    subplot(2,1,1)
    plot(rad2deg(alpha_rudder), F_lift_v_right, '+r', rad2deg(alpha_rudder), F_drag_v_right, 'o')
    hold on
    subplot(2,1,2)
    plot(rad2deg(alpha_rudder), X_rudder, 'r', rad2deg(alpha_rudder), Y_rudder,'b', rad2deg(alpha_rudder), N_rudder, 'g')
    hold on
    
end

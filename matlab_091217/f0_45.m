% Numerical intergration
% unchangeable variables
% -------------------------------------------------------------------------
function y=f0_45(alpha_rudder_r, torque_des, vel, pose, d_wind, v_wind, aoa)
delta_t         = 0.2;
dens_air        = 1.23;             % [kg/m^3] density of air
dens_water      = 1024;             % [kg/m^3] density of water
g               = 9.81;              % [m/s^2]

A_hull(3)       = 2;            C_d(3)      = 1.3;        width  = 1.2;
A_sail          = 8.4;
A_rudder        = 0.085;
I(3)            = 150;

% forces
% -------------------------------------------------------------------------
% damping terms (depends on speed of boat, Hull resistance)
% -------------------------------------------------------------------------
N_damping   = sign(vel(3,1))*A_hull(3)*0.5*dens_water*C_d(3)*(width/2*vel(3,1))^2;
N_damp_rot  = sign(vel(3,1))*1600*(width/2*vel(3,1)^2);
N_damping   = N_damping + N_damp_rot; % - 1*Y_lateral;Y_lateral  = sign(vel(2,1))*A_keel*0.5*dens_water*c_d_keel*vel(2,1)^2;

% current  -> neglected
% -------------------------------------------------------------------------
% wind     -> neglected
% -------------------------------------------------------------------------
% waves    -> neglected
% ---------------------------------------------------------------------
% sail force
% -------------------------------------------------------------------------
u_r = v_wind*cos(d_wind-pose(3)-pi) - vel(1,1);% - u_c;      
v_r = v_wind*sin(d_wind-pose(3)-pi) - vel(2,1);% - v_c;      

V_wind = sqrt(u_r^2 + v_r^2);

d_wind_r = d_wind - pose(3); %g_r-pi;
d_wind_r = reminderRad(d_wind_r);
d2aoa = reminderRad((-d_wind_r+aoa));
    if abs(d2aoa) <= 2*pi/180             % Hysteresis around "dead into the wind"
        c_sail_lift = 0;
    elseif abs(d2aoa) <= 25*pi/180        % 25deg = max c_sail_lift
        c_sail_lift = (2.24*abs(d2aoa)-2.24*2*pi/180);  % sign(aoa)*
    elseif (abs(d2aoa) > 25*pi/180) && (abs(d2aoa) <= 90*pi/180)
        c_sail_lift = -0.79*abs(d2aoa)+1.1;           %sign(aoa)*(-0.79*abs(aoa)+0.9)
    elseif  (abs(d2aoa) > 90*pi/180) && (abs(d2aoa) <= pi)
        c_sail_lift = 0;
    end 
c_sail_drag     = 1.28*sin(abs(-d_wind_r+aoa));
% -------------------------------------------------------------------------
F_lift_V_w      = 1/2*dens_air*c_sail_lift*V_wind^2*A_sail*cos(-d_wind_r+aoa);
F_drag_V_w      = 1/2*dens_air*c_sail_drag*V_wind^2*A_sail*sin(-d_wind_r+aoa)*sign(-d_wind_r+aoa);
% -------------------------------------------------------------------------
if d_wind_r >= 0
    vorzeichen = 1;
else
    vorzeichen = -1;
end
% distance from cog to coa of sail for N (moment around z);
if abs(aoa) <= 2*pi/180
    x_to_sail_coa = 0; y_to_sail_coa = 0;
elseif abs(aoa) <= 40*pi/180
    x_to_sail_coa = 0.2; y_to_sail_coa = 0.3;
elseif abs(aoa) > 40*pi/180 && abs(aoa) <= 70*pi/180
    x_to_sail_coa = 0.3; y_to_sail_coa = 0.35;
elseif abs(aoa) > 70*pi/180 && abs(aoa) <= 120*pi/180
    x_to_sail_coa = 0.4; y_to_sail_coa = 0.4;
elseif abs(aoa) > 120*pi/180 && abs(aoa) <= 150*pi/180
    x_to_sail_coa = 0.2; y_to_sail_coa = 0.3;
elseif abs(aoa) > 150*pi/180 && abs(aoa) <= pi
    x_to_sail_coa = 0; y_to_sail_coa = 0;
end
% -------------------------------------------------------------------------
X_sail = F_lift_V_w*sin(abs(d_wind_r)) - F_drag_V_w*cos(d_wind_r);
Y_sail = (F_lift_V_w*cos(d_wind_r) + F_drag_V_w*sin(abs(d_wind_r)))*sign(-vorzeichen);
N_sail = X_sail*x_to_sail_coa*sign(aoa) + Y_sail*y_to_sail_coa;

N_sail = 0.1*N_sail;

% numerical solution
% -------------------------------------------------------------------------
% -------------------------------------------------------------------------
% -------------------------------------------------------------------------
% N_des           = (theta_dot_des - vel(3,1))*I(3)/delta_t;
N_des           = torque_des*0.9;
N_rudder_des    =  N_sail - N_damping - N_des;

Y_rudder        = N_rudder_des/1.7;
Y_rudder_right  = Y_rudder/2;

rot = [0;0;vel(3,1)];
dist_to_rudder = [-1.7; 0; 0];
vel_app_rudder = vel + cross(rot,dist_to_rudder);
v_r_tot = sqrt((vel_app_rudder(1))^2 + (vel_app_rudder(2))^2);% + (vel(3,1)*1.7)^2);

d_water = atan2(vel_app_rudder(2)*0.01,vel_app_rudder(1));% 
d_water = reminderRad(d_water);

k               = 1/2*dens_water*v_r_tot^2*A_rudder;

% solve 
% ------------------------------------------------------------------------
incid_angle     = -d_water + alpha_rudder_r;
incid_angle     = reminderRad(incid_angle);
%    c_rudder_lift_appr =
%    1.9*(1-exp(-abs(alpha_rudder)*9))-2.4*abs(alpha_rudder);
if incid_angle >= 0
    vorzeichenR = 1;
else
    vorzeichenR = -1;
end

if d_water >= 0
    vorzeichenRR = 1;
else
    vorzeichenRR = -1;
end

 % z = rad2deg(fzero(@f2_25,[deg2rad(2) deg2rad(25)]))
% for (-d_water + alpha_rudder) = [0 - 2*pi/180]
y   = k*(1.9*(1-exp(-abs(reminderRad(-d_water + alpha_rudder_r))*9))-2.4*abs(reminderRad(-d_water + alpha_rudder_r)))*cos(-d_water+alpha_rudder_r)*cos(d_water)*sign(vorzeichenR) + k*1.28*sin(abs(reminderRad(-d_water + alpha_rudder_r)))*sin(-d_water+alpha_rudder_r)*sign(vorzeichenR)*sin(abs(d_water))*sign(-vorzeichenRR) - Y_rudder_right;
% root = roots(y)
end



% Mathematical model of the movement of the ship in the water
% with influence of wind, current and waves
%
% - Position update
%
% (c) by Fabian Jenne
% see documentation in "Identification and adaptive control applied to
% ship steering" by Claes G K�llstr�m
% and "Endbericht Final Avalon Team"



function [pose, vel_p, vel, X, Y, N, X_p, Y_p, N_p, X_drag, Y_drag, X_waves, Y_waves, N_waves, X_sail, Y_sail, N_sail, N_rudder, N_damping, V_wind, g_r] = PoseStep(t, delta_t, pose, vel, X_p, Y_p, N_p, X_drag, Y_drag,vel_p, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, v_current, d_current, v_wind, d_wind, d_waves, T, h, depth, length, width, sail_factor)

% unchangeable variables
% -------------------------------------------------------------------------

dens_air = 1.23;            % [kg/m^3] density of air
dens_water = 1024;          % [kg/m^3] density of water
% g =9.81;                    % [m/s^2]

% forces
% -------------------------------------------------------------------------
% damping terms (depends on speed of boat, Hull resistance)
% -------------------------------------------------------------------------
% A_hull(3) = A_hull3;
X_damping = sign(vel(1,1))*A_hull(1)*0.5*dens_water*C_d(1)*vel(1,1)^2;
Y_damping = sign(vel(2,1))*A_hull(2)*0.5*dens_water*C_d(2)*vel(2,1)^2;
N_damping = sign(vel(3,1))*A_hull(3)*0.5*dens_water*C_d(3)*(width/2*vel(3,1))^2;
% N_damping = 1.3*N_damping;
A_keel = 0.5; c_d_keel = 1.28;
Y_lateral  = sign(vel(2,1))*A_keel*0.5*dens_water*c_d_keel*vel(2,1)^2;
N_damp_rot = sign(vel(3,1))*1600*(width/2*vel(3,1)^2); % *1600

Y_damping = Y_damping + Y_lateral;
N_damping = N_damping + N_damp_rot;% - 1*Y_lateral;
% current
% -------------------------------------------------------------------------

% u_c = v_current*cos(d_current-pose(3)-pi);
% v_c = v_current*sin(d_current-pose(3)-pi);
% 
% X_current = m*v_c*vel(3,1);
% Y_current = -m*u_c*vel(3,1);   %Nach Buch minus,...
% N_current = 0;
% 
% small=0;
% X_current = small*X_current;
% Y_current = small*Y_current;
% N_current = small*N_current;
X_current = 0;
Y_current = 0;
N_current = 0;
% 
% % wind
% % -------------------------------------------------------------------------
% % u_r = v_wind*cos(d_wind-pose(3)-pi) - vel(1,1) - u_c;       %(d_wind-pose(3)-pi) without data from windprofile
% % v_r = v_wind*sin(d_wind-pose(3)-pi) - vel(2,1) - v_c;       %Nach Buch minus ( - vel(1)), macht aber keinen Sinn da Kr�fte zunehmen!  
% % 
% % V_wind = sqrt(u_r^2 + v_r^2);
% % g_r = atan2(v_r,u_r);
% % 
% % if u_r >= 0
% %     g_r = g_r + pi;
% % elseif u_r < 0 && v_r > 0
% %     g_r = g_r + 2*pi;
V_wind_x=(v_wind*cos(d_wind-pose(3))+vel(1));
V_wind_y=(v_wind*sin(d_wind-pose(3))+vel(2));
V_wind=sqrt((V_wind_x)^2+(V_wind_y)^2);
g_r=reminderRad(atan2(V_wind_y,V_wind_x)-aoa_sail);
% % [d_wind g_r]
% % V_wind=v_wind;
% % g_r=d_wind;
% % % end
% % % 
% % % while g_r > pi
% % %     g_r = g_r-pi;
% % % end
% % % 
% % % while g_r < -pi
% % %     g_r = g_r+pi;
% % % end
% % g_r = reminderRad(g_r);
% 
% C_X = C_hat(1)*cos(g_r);
% C_Y = C_hat(2)*sin(g_r);
% 
% if g_r > pi/2 && g_r < 3*pi/2
%     C_N_hat = C_hat(4);
% else
%     C_N_hat = C_hat(3);
% end
% 
% C_N = C_N_hat*sin(2*g_r);
% 
% X_wind = 1/2*dens_air*V_wind^2*C_X*length^2;
% Y_wind = 1/2*dens_air*V_wind^2*C_Y*length^2;
% N_wind = 1/2*dens_air*V_wind^2*C_N*length^3;
% %N_wind = 0.1 * N_wind;
% X_wind = 0.3*X_wind; % 0.6
% Y_wind = 0.3*Y_wind;
X_wind = 0; % 0.6
Y_wind = 0;
N_wind = 0;
%    waves
%    ---------------------------------------------------------------------
% d_waves = reminderRad(d_waves);
% 
% w = 2*pi/T;
% k = 4*pi^2/(g*T^2);
% 
% chi = pi - (d_waves-pose(3));
% w_e = w - k*vel(1,1)*cos(chi)+k*vel(2,1)*sin(chi);
% a = dens_water*g*(1-exp(-k*depth))/k^2;
% b = k*length/2*cos(chi);
% c = k*width/2*sin(chi);
% s = (k*h/2)*sin(w_e*t);
% eps = h/2*cos(w_e*t);
% 
% if c==0, c=0.001; end
% if b==0, b=0.001; end                   %not so nice (c=0 or b=0)
% 
% % X_waves = 2*a*width*sin(b)*sin(c)/c*s;     
% % Y_waves = -2*a*length*sin(b)*sin(c)/b*s;
% % N_waves = a*k*(width^2*sin(b)*(c*cos(c)-sin(c))/c^2-length^2*sin(c)*(b*cos(b)-sin(b))/b^2)*eps*1/6;   % correction term: *small
% %X_waves = 0;
% %Y_waves = 0;
% % N_waves = 0;
% c_d_hull_wave_x = 0.1; % (un)educated guess, tried to get maximum speed to realistic value
% c_d_hull_wave_y = 5000;

X_waves = 0;%sign(vel(1,1))*A_hull(1)*0.5*dens_water*c_d_hull_wave_x*vel(1,1)^4;
Y_waves = 0;%sign(vel(2,1))*A_hull(2)*0.5*dens_water*c_d_hull_wave_y*vel(2,1)^4;
N_waves = 0;
% sail force
% -------------------------------------------------------------------------

                                    % gerundet = ceil(t/20);
                                    % modulo_m = mod(gerundet,2);
                                    % vorzeichen = (-1)^(modulo_m+1);
                                    % aoa = vorzeichen * deg2rad(aoa_sail);
aoa = aoa_sail;
d_wind_r = d_wind - pose(3); %g_r-pi;
d_wind_r = reminderRad(d_wind_r);

% [aoa d_wind_r];
d2aoa = reminderRad((-d_wind_r+aoa));
% [aoa d2aoa d_wind_r];
    if abs(d2aoa) <= 2*pi/180             % Hysteresis around "dead into the wind"
        c_sail_lift = 0;
    elseif abs(d2aoa) <= 25*pi/180        % 25deg = max c_sail_lift
        c_sail_lift = (2.24*abs(d2aoa)-2.24*2*pi/180);  % sign(aoa)*
    elseif (abs(d2aoa) > 25*pi/180) && (abs(d2aoa) <= 90*pi/180)
        c_sail_lift = -0.79*abs(d2aoa)+1.1;           %sign(aoa)*(-0.79*abs(aoa)+0.9)
    elseif  (abs(d2aoa) > 90*pi/180) && (abs(d2aoa) <= pi)
        c_sail_lift = 0;
    end
    
c_sail_drag = 1.28*sin(abs(-d_wind_r+aoa));
F_lift_V_w = 1/2*dens_air*c_sail_lift*V_wind^2*A_sail*cos(-d_wind_r+aoa);%*sign(-d_wind-aoa);
F_drag_V_w = 1/2*dens_air*c_sail_drag*V_wind^2*A_sail*sin(-d_wind_r+aoa)*sign(-d_wind_r+aoa);
% [c_sail_lift c_sail_drag F_lift_V_w F_drag_V_w];
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

X_sail = F_lift_V_w*sin(abs(d_wind_r)) - F_drag_V_w*cos(d_wind_r);
Y_sail = (F_lift_V_w*cos(d_wind_r) + F_drag_V_w*sin(abs(d_wind_r)))*sign(-vorzeichen);
N_sail = X_sail*x_to_sail_coa*sign(aoa) + Y_sail*y_to_sail_coa;
N_sail = sail_factor*N_sail;
% X_sail = 200;
% Y_sail =0;
% N_sail = 0;
% rudder
% ------------------------------------------------------------------------

alpha_rudder = -alpha_rudder_r;
% velr(1,1) = vel(1,1);
% if pose(3) > pi/2 || pose(3) < -pi/2
%     velr(1,1) = -velr(1,1);
% end
rot = [0;0;vel(3,1)];
dist_to_rudder = [-1.7; 0; 0];
vel_app_rudder = vel + cross(rot,dist_to_rudder);
v_r_tot = sqrt((vel_app_rudder(1))^2 + (vel_app_rudder(2))^2);% + (vel(3,1)*1.7)^2);

d_water = atan2(vel_app_rudder(2)*0.01,vel_app_rudder(1));% 
d_water = reminderRad(d_water);

% c_rudder_drag = 1.28*sin(abs(-d_water + alpha_rudder));

incid_angle = -d_water + alpha_rudder;
incid_angle = reminderRad(incid_angle);
format long
% [v_r_tot d_water incid_angle alpha_rudder];
% if abs(incid_angle) <= 2*pi/180            % Hysteresis around "dead into the wind"
%     c_rudder_lift = 0;
% elseif abs(incid_angle) <= 25*pi/180       % 25deg = max c_sail_lift
%     c_rudder_lift = (2*abs(incid_angle)-2*2*pi/180);   % 2.24
% elseif abs(incid_angle) > 25*pi/180 && abs(incid_angle) < 90*pi/180
%     c_rudder_lift = -0.79*abs(incid_angle)+0.9; % sign(alpha_rudder)*(-0.79*abs(alpha_rudder)+0.9)    
% elseif abs(incid_angle) >= 90*pi/180 && abs(incid_angle) <= pi
%     c_rudder_lift = 0;
% end
c_rudder_drag = 0.1+0.3*incid_angle^2; % from Mario
% c_rudder_lift = abs(abs(7.1*incid_angle) - 4*incid_angle^2 -abs(16.6*incid_angle^3)); % from Mario
% c_rudder_drag = 0;%0.2+0.3*incid_angle^2; % from Mario
% c_rudder_lift = abs(abs(7.1*incid_angle) - 4*incid_angle^2 -abs(16.6*incid_angle^3)); % from Mario
c_rudder_lift = abs(.9*incid_angle);
% c_rudder_lift = 1.9*(1-exp(-abs(incid_angle)*9))-2.4*abs(incid_angle);
% [c_rudder_drag c_rudder_lift, incid_angle];
if incid_angle > 0
    vorzeichenR = 1;
else
    vorzeichenR = -1;
end
    
F_lift_v_right = 1/2*dens_water*c_rudder_lift*v_r_tot^2*A_rudder*cos(incid_angle);
F_drag_v_right = 1/2*dens_water*c_rudder_drag*v_r_tot^2*A_rudder*sin(incid_angle)*sign(vorzeichenR);

F_lift_v_left = 1/2*dens_water*c_rudder_lift*v_r_tot^2*A_rudder*cos(incid_angle);
F_drag_v_left = 1/2*dens_water*c_rudder_drag*v_r_tot^2*A_rudder*sin(incid_angle)*sign(vorzeichenR);

% if d_water >= 0
%     vorzeichenRR = 1;
% else
%     vorzeichenRR = -1;
% end
X_rudder_right = F_lift_v_right*sin(abs(d_water)) - F_drag_v_right*cos(d_water);
Y_rudder_right = F_lift_v_right*cos(d_water)*sign(vorzeichenR) + F_drag_v_right*sin(-d_water);% )*sign(-vorzeichenRR); %sign(vorzeichenR)....))*sign(-vorzeichenRR)

X_rudder_left = F_lift_v_left*sin(abs(d_water)) - F_drag_v_left*cos(d_water);
Y_rudder_left = F_lift_v_left*cos(d_water)*sign(vorzeichenR) + F_drag_v_left*sin(-d_water);

X_rudder = X_rudder_left + X_rudder_right;
Y_rudder = Y_rudder_left + Y_rudder_right;
N_rudder = -Y_rudder*1.7;        % 2*Y_rudder ????    distance CoR to rudder-center = -1.7 m (in x-direction)
%N_rudder = 0.4*N_rudder;
% summarize the forces/moments
% -------------------------------------------------------------------------
% N_sail=0;
X = X_sail + X_current + X_wind + X_waves + X_rudder - X_damping;   
Y = Y_sail + Y_current + Y_wind + Y_waves + Y_rudder - Y_damping;
N = N_sail + N_current + N_wind + N_waves + N_rudder - N_damping;% -N_rudder
N = N_rudder;
N;
% [aoa d_wind_r N_sail];
% [N N_damping N_sail N_rudder];
N_rudder;
0;
% N = 0.9*N;                                                              % 0.6 correction term by FJ   waves must be additive

%str=['N_rud:' num2str(N_rudder) '   N_sail:' num2str(N_sail) ' N_wind:' num2str(N_wind) '   N:' num2str(N)]; 
%disp(str)
X_p = [X_p X];
Y_p = [Y_p Y];
N_p = [N_p N];
X_drag = [X_drag F_drag_V_w];
Y_drag = [Y_drag F_lift_V_w];
% [vel(3) N vel(1) vel(2) aoa d_wind pose(3) v_wind];
% discret velocity update
% -------------------------------------------------------------------------
% I(3)            = I_z;
vel_new         = vel;
vel_new(3,1)    = delta_t/I(3)*N+vel(3,1);
if vel_new(3,1)>0
    vel_new(3,1)=min(vel_new(3,1),.09);
else
    vel_new(3,1)=max(vel_new(3,1),-.09);
end
vel_new(2,1)    = 1/(1+vel_new(3,1)*delta_t^2)*(Y/m*delta_t+vel(2,1)-delta_t^2*X/m);
vel_new(1,1)    = delta_t*(X/m+vel_new(2,1)*vel_new(3,1))+vel(1,1);
vel             = vel_new;
vel([1:2])      = [6;0.03];
vel_p           = [vel_p vel];

pose(3)         = reminderRad(pose(3));
psi             = pose(3);
R               = [cos(psi) -sin(psi) 0; sin(psi) cos(psi) 0; 0 0 1];

pose            = pose + (R*vel)*delta_t;
% [vel(3) N alpha_rudder*180/pi];
end


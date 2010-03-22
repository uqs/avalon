%
% Mathematical model of the movement of the ship in the water
% with influence of wind, current and waves
%
% (c) by Oliver Baur
% see documentation in ship3

clear all; close all; clc;

% changeable variables 
% ---------------------------------
% simulation variables
T_sim = 50;                 % [s] time of simulation
delta_t = 1/15;             % [s] iteration time step (smallest time step of sensor reading)
world_size = 100;           % [m] size of the part we are looking at
disp_forces = 1;            % display forces; 0 or 1
disp_measurements = 0;      % display measurements; 0 or 1
%pos_control = 1;            % position control; 0 or 1
%pathpoints = [-30,-30,0;-25,10,0.5*pi;10,20,pi;0,0,0];

%
% Mathematical model of the movement of the ship in the water
% with influence of wind, current and waves
%
% - Parameters
%
% (c) by Fabian Jenne
% see documentation in "Identification and adaptive control applied to
% ship steering" by Claes G K�llstr�m
% and "Endbericht Final Avalon Team"


% unchangeable variables
% ---------------------------------Sail section lift
dens_air = 1.23;                % [kg/m^3] density of air
dens_water = 1025;              % [kg/m^3] density of water
g =9.81;                        % [m/s^2]

% changeable variables 
% ---------------------------------
% initial kinematic variables
vel = [0;0;0];                  % u [m/s], v [m/s], r [rad/s]
pose = [0;0;deg2rad(0)];            % x [m], y [m], psi [rad] start pose

% boat variables 
m = 450;                        % [kg] mass of the ship
length = 3.95; %4                  % [m] length of ship
width = 1.5;                    % [m] width of ship parts in water
depth = 0.2;                    % [m] depth of ship parts in water
I = [635;2427;150]; %150 for Iz             % [kg*m^2] moment of inertia

A_sail = 8.4;                   % [m^2] Area of the sail
A_hull = [0.3;0.8;2];         % [m^2] Area for hull drag
C_d = [0.4;1.2;1.2];            % damping coefficient, for hull drag
C_hat = [-0.007;-0.016;-0;-0];  % wind coefficient, see page 36

% environment variabels
% ---------------------------------
% current
v_current = 2;                % [m/s] current velocity
d_current = deg2rad(2);            % [rad] current direction, 180�: current from behind the ship

% wind
v_wind = 8;                   % [m/s] wind velocity
d_wind = deg2rad(0);               % [rad] wind direction, 180�: wind from behind

% sail angle! ! !! !!!!!!!!!!!!!!!!!!!!!!!!!!
a_sail = deg2rad(20);                                    % [rad] Angle of Attack, optimum : aoa = 20� (by cedric)

% waves
d_waves = deg2rad(2);              % [rad] wave direction (180�: waves coming from behind
h = 2;                          % [m] wave height
T = 8;                          % [s] wave period, MUST BE LONGER, THEN THE SHIP LENGTH -> unstable otherwise



% sensors
sigma_gps = 2.5*1.2;        % [m] CEP to RMS = x1.2 (see http://en.wikipedia.org/wiki/Circular_error_probable)
update_gps = 1/4;           % [s]
sigma_compass = 4/180*pi;   % [rad]
update_compass = 1/15;      % [s]

% % kalman filter variables
% % ---------------------------------
% phi = [1 0 0 update_gps 0 0;        % 1st order kalman filter
%        0 1 0 0 update_gps 0;
%        0 0 1 0 0 update_gps;
%        0 0 0 1 0 0;
%        0 0 0 0 1 0;
%        0 0 0 0 0 1];
% gamma = 0.01;  % process noise: Q->0: 100% trust in model, Q->inf, 100% trust in measurements
% 
% Q = gamma*[update_gps^3/3 0 0 update_gps^2/2 0 0;
%            0 update_gps^3/3 0 0 update_gps^2/2 0;
%            0 0 update_gps^3/3 0 0 update_gps^2/2;
%            update_gps^2/2 0 0 update_gps 0 0;
%            0 update_gps^2/2 0 0 update_gps 0;
%            0 0 update_gps^2/2 0 0 update_gps];
% H = [1,0,0,0,0,0;
%      0,1,0,0,0,0;
%      0,0,1,0,0,0];                  %observation matrix
% R = [sigma_gps^2 0 0;
%      0 sigma_gps^2 0;
%      0 0 sigma_compass^2];            % measurement uncertainty matrix
% P = 1000*eye(6);                    % state estimation covariance matrix
% G = [0 0 0;
%      0 0 0;
%      0 0 0;
%      1 0 0;
%      0 1 0;
%      0 0 1];                        % control matrix
% u = [0;0;0];
% x_hat = [0;0;0;0;0;0];              % x,y,psi,x_dot,y_dot,psi_dot - all in earth frame
% 
% % position control variables
% % ---------------------------------
% kr=3;
% ka=8;
% kb=-1.5;





% program
% ---------------------------------
t=0;
X_p = [];
Y_p = [];
N_p = [];
vel_p = [];
figure(1);
% t_gps = 100;
% t_compass = 100;
% gi=1;
% goal = pathpoints(gi,:);
% stop_sim = 0;

% P_p = [];
% Err_p = [];
    
n = 1;

while t < T_sim

    % forces
    % -------------------------------------------------------------------------    
    % damping terms (depends on speed of boat, Hull resistance)
    % ---------------------------------------------------------------------
    X_damping = sign(vel(1))*A_hull(1)*0.5*dens_water*C_d(1)*vel(1)^2;
    Y_damping = sign(vel(2))*A_hull(2)*0.5*dens_water*C_d(2)*vel(2)^2;
    N_damping = 0*sign(vel(3))*A_hull(3)*0.5*dens_water*C_d(3)*vel(3)^2;
%     X_damping = 0;
%     Y_damping = 0;
%     N_damping = 0;
%     
    % current
    % ---------------------------------------------------------------------
    u_c = v_current*cos(d_current-pose(3)-pi);
    v_c = v_current*sin(d_current-pose(3)-pi);
    
    X_current = m*v_c*vel(3);
    Y_current = -m*u_c*vel(3);   %Nach Buch minus,...
    N_current = 0;
    
%     X_current = 0;
%     Y_current = 0;
%     N_current = 0;

    % wind
    % ---------------------------------------------------------------------
    %u_r = v_wind*cos(d_wind-pose(3)) + vel(1) + u_c;         %Nach Buch minus, macht aber keinen Sinn da Kr�fte zunehmen!
    %v_r = v_wind*sin(d_wind-pose(3)) + vel(2) + v_c;

    u_r = v_wind*cos(d_wind-pose(3)-pi) + vel(1) + u_c; 
    v_r = v_wind*sin(d_wind-pose(3)-pi) + vel(2) + v_c;
    
    V_wind = sqrt(u_r^2 + v_r^2);

    g_r = atan2(v_r,u_r);
    
    while g_r > pi
        g_r = g_r-pi;
    end
    
    while g_r < -pi
        g_r = g_r+pi;
    end
    
    C_X = C_hat(1)*cos(g_r);
    C_Y = C_hat(2)*sin(g_r);
    
    if g_r > pi/2 && g_r < 3*pi/2
        C_N_hat = C_hat(4);
    else
        C_N_hat = C_hat(3);
    end
    
    C_N = C_N_hat*sin(2*g_r);

    X_wind = 1/2*dens_air*V_wind^2*C_X*length^2;
    Y_wind = 1/2*dens_air*V_wind^2*C_Y*length^2;
    N_wind = 1/2*dens_air*V_wind^2*C_N*length^3;
    
%     X_wind = 0;
%     Y_wind = 0;
%     N_wind = 0;

%    waves
%    -----------------------------------------------------------------    
    w = 2*pi/T;
    k = 4*pi^2/(g*T^2);

    chi = pi - (d_waves-pose(3));                   
    w_e = w - k*vel(1)*cos(chi)+k*vel(2)*sin(chi); 
    a = dens_water*g*(1-exp(-k*depth))/k^2;
    b = k*length/2*cos(chi);
    c = k*width/2*sin(chi);
    s = (k*h/2)*sin(w_e*t);
    eps = h/2*cos(w_e*t);

%       if c==0, c=0.001; end
%       if b==0, b=0.001; end
    X_waves = 2*a*width*sin(b)*sin(c)/c*s;     %not so nice (c=0 or b=0)
    Y_waves = -2*a*length*sin(b)*sin(c)/b*s;
    N_waves = a*k*(width^2*sin(b)*(c*cos(c)-sin(c))/c^2 - length^2*sin(c)*(b*cos(b)-sin(b))/b^2)*eps*1/6;   % correction term: *1/6

%     X_waves = 0;
%     Y_waves = 0;
%     N_waves = 0;


    % sail force
    % ---------------------------------------------------------------------

%         gerundet = ceil(t/10);
%         modulo_m = mod(gerundet,2);
%         vorzeichen = (-1)^(modulo_m+1);
%         aoa = vorzeichen * a_sail;
   aoa = deg2rad(20);
    % Coefficients of lift and drag:
    % Lift: aproximation taken from "Briere, Y.: IBOAT: An autonomous robot for long-term offshore
    % operation."
    if abs(aoa) <= 2*pi/180 % Hysteresis around "dead into the wind"
        c_sail_lift = 0;
    elseif abs(aoa) <= 25*pi/180 % 25deg = max c_sail_lift
        c_sail_lift = (2.24*abs(aoa)-2.24*2*pi/180);            % sign(aoa)* ......
    elseif abs(aoa) > 25*pi/180 && abs(aoa) < 90*pi/180
        c_sail_lift = (-0.79*abs(aoa)+0.9);                     % sign(aoa)* ......
    else % abs(alpha_sail) >= 90*pi/180 && abs(alpha_sail) <= pi
        c_sail_lift = 0;
    end
    
    c_sail_drag=1.28*sin(abs(aoa));
    V_wind_relative = sqrt(V_wind^2 + (vel(1))^2 + 2*V_wind*vel(1)*cos(d_wind));
    
    X_sail = 1/2*dens_air*u_r^2*A_sail*c_sail_lift*sin(aoa);   %( d_wind-pose(3) ) instead of g_r
    Y_sail = 1/2*dens_air*v_r^2*A_sail*c_sail_drag*cos(aoa);
    N_sail = 0;
    
    % rudder
    % ---------------------------------------------------------------------
    A_rudder = 0.085;                           % in m^2. 0.17*0.5: Surface of ONE rudder
    alpha_rudder = deg2rad(0);
    
    % Coefficients of lift and drag:
    % Lift: aproximation taken from "Briere, Y.: IBOAT: An autonomous robot for long-term offshore
    % operation."
    if abs(alpha_rudder) <= 2*pi/180 % Hysteresis around "dead into the wind"
        c_rudder_lift = 0;
    elseif abs(alpha_rudder) <= 25*pi/180 % 25deg = max c_sail_lift
        c_rudder_lift = sign(alpha_rudder)*(2.24*abs(alpha_rudder)-2.24*2*pi/180);
    elseif abs(alpha_rudder) > 25*pi/180 && abs(alpha_rudder) < 90*pi/180
        c_rudder_lift = sign(alpha_rudder)*(-0.79*abs(alpha_rudder)+0.9);
    else % abs(alpha_rudder) >= 90*pi/180 && abs(alpha_rudder) <= pi
        c_rudder_lift = 0;
    end
    
    c_rudder_drag=1.28*sin(abs(alpha_rudder));
    
    X_rudder_single = 1/2*dens_water*c_rudder_lift*(vel(1)-u_c)^2*sin(alpha_rudder)*A_rudder; %sin(alpha_ruder) selbst hinzugef�gt
    Y_rudder_single = 1/2*dens_water*c_rudder_drag*(vel(2)-v_c)^2*cos(alpha_rudder)*A_rudder;
    
    X_rudder = 2*X_rudder_single;
    Y_rudder = 2*Y_rudder_single;
    N_rudder = -Y_rudder*(length/2-0.3);  %distance has to be determined percisely
    
    % summarize the forces/moments
    % ---------------------------------------------------------------------
    X = -X_sail + X_current + X_wind + X_waves - X_rudder - X_damping;
    Y = Y_sail + Y_current + Y_wind + Y_waves - Y_rudder - Y_damping;
    N = N_sail + N_current + N_wind + N_waves + N_rudder - N_damping;
    
    X_p = [X_p X];
    Y_p = [Y_p Y];
    N_p = [N_p N];
    
    %discret velocity update
    % ---------------------------------------------------------------------
    vel_new = vel;
    vel_new(3) = delta_t/I(3)*N+vel(3);
    vel_new(2) = 1/(1+vel_new(3)*delta_t^2)*(Y/m*delta_t+vel(2)-delta_t^2*X/m);
    vel_new(1) = delta_t*(X/m+vel_new(2)*vel_new(3))+vel(1);
    vel = vel_new;
    vel_p = [vel_p vel];
    
    psi = pose(3);
    R = [cos(psi) -sin(psi) 0; sin(psi) cos(psi) 0; 0 0 1];
    pose = pose + R*vel*delta_t;
    
    
    %% Plotting
    traj(n,:)=[pose(1),pose(2)];
    n=n+1;    
    
    % plotting
    % ---------------------------------

    if abs(pose(1)) > world_size/2
        world_size = abs(pose(1))*2;
    end
    if abs(pose(2)) > world_size/2
        world_size = abs(pose(2))*2;
    end
    
    psi = pose(3);
    boat = [ones(1,5)*pose(1); ones(1,5)*pose(2)] + ([0,-length/2, +length/2, -length/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(psi) sin(psi); -sin(psi) cos(psi)])';
    wind = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    waves = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_waves+pi) sin(d_waves+pi); -sin(d_waves+pi) cos(d_waves+pi)])';


    if disp_forces == 1
        f = sqrt(X^2+Y^2);
        d_f = atan2(Y,X)+psi;
        forces = [ones(1,5)*pose(1); ones(1,5)*pose(2)] + ([0,+f, +f/2, f, f/2; 0, 0, -f/2, 0, f/2]'*[cos(d_f) sin(d_f); -sin(d_f) cos(d_f)])';
        moment = [];
        if N < 0 
            for i=0:-pi/16:-pi
                mom = [pose(1)+N*cos(i); pose(2)+N*sin(i)];
                moment = [moment mom];
            end
            mom = [pose(1)+N/2*cos(-pi*2/3) pose(1)+N*cos(-pi) pose(1)+3/2*N*cos(-pi*2/3); pose(2)+N/2*sin(-pi*2/3) pose(2)+N*sin(-pi) pose(2)+3/2*N*sin(-pi*2/3)];
            moment = [moment mom];
        else
            for i=0:pi/16:pi
                mom = [pose(1)+N*cos(i); pose(2)+N*sin(i)];
                moment = [moment mom];
            end
            mom = [pose(1)+N/2*cos(pi*2/3) pose(1)+N*cos(pi) pose(1)+3/2*N*cos(pi*2/3); pose(2)+N/2*sin(pi*2/3) pose(2)+N*sin(pi) pose(2)+3/2*N*sin(pi*2/3)];
            moment = [moment mom];
        end
    end

%     if disp_measurements == 1
%         if t_gps >= update_gps
%             gps_length = random('norm',0,sigma_gps);
%             gps_orient = random('unif',0,2*pi);
%             t_gps = 0;
%         end
%         if t_compass >= update_compass
%             compass_orient = random('norm',0,sigma_compass);
%             t_compass = 0;
%         end
%         sens_point = pose + [gps_length*cos(gps_orient); gps_length*sin(gps_orient); compass_orient];
%         measurement = [ones(1,5)*sens_point(1); ones(1,5)*sens_point(2)] + ([0,-length/2, +length/2, -length/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(sens_point(3)) sin(sens_point(3)); -sin(sens_point(3)) cos(sens_point(3))])';
%     end
    
    if disp_forces == 1
        if disp_measurements == 1
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g',measurement(2,:),measurement(1,:),'r');      %coordinate system is other way round
        else
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g');      %coordinate system is other way round
        end
    else
        if disp_measurements == 1
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',measurement(2,:),measurement(1,:),'r');      %coordinate system is other way round
        else
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g');      %coordinate system is other way round
        end
    end
    axis([-world_size/2 world_size/2 -world_size/2 world_size/2]);

%     title([num2str(pose(1)),', ', num2str(pose(2)), ', ', num2str(pose(3)*180/pi),', Windwinkel zum Boot = ',num2str(g_r*180/pi),'�',', chi = ',num2str(chi/pi*180)]);
    title(['Simulation time = ',num2str(t),' s']);
    drawnow;
    pause(delta_t);
          
    t=t+delta_t;
end


%t=t-delta_t;
figure(2);
plot(0:delta_t:t,X_p,'r',0:delta_t:t,Y_p,'g',0:delta_t:t,N_p,'b');
title('forces');

figure(3);
plot(0:delta_t:t,vel_p(1,:),'r',0:delta_t:t,vel_p(2,:),'g',0:delta_t:t,vel_p(3,:),'b');
title('velocities');

figure(4);
plot(traj(:,1),traj(:,2));
title('trajectory');
  
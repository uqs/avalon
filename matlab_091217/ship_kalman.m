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
disp_forces = 0;            % display forces; 0 or 1
disp_measurements = 1;      % display measurements; 0 or 1
pos_control = 1;            % position control; 0 or 1
pathpoints = [-30,-30,0;-25,10,0.5*pi;10,20,pi;0,0,0];

% boat variables 
vel = [0;0;0];              % u [m/s], v [m/s], r [rad/s]
thrust1 = 0.0;              % between (-1) and 1, thrust level engine 1
thrust2 = 0.0;              % between (-1) and 1, thrust level engine 2
pose = [0;0;pi*0.25];        % x [m], y [m], psi [rad]
goal = [-30;-30;0];           % x [m], y [m], psi [rad]

% environment variabels
v_wind = 20.0;                % [m/s] wind velocity
d_wind = 0.25*pi;              % [rad] wind direction
d_waves = 0.00*pi;               % [rad] wave direction
h = 1.5;                    % [m] wave height
T = 4;                      % [s] wave period

% ungchangeable variables
% ---------------------------------
dens_air = 1.23;            % [kg/m^3] Density of air
dens_water = 1000;          % [kg/m^3] Density of water
g =9.81;                    % [m/s^2]

% variables, still to be determined
% ---------------------------------
% boat
m = 100;                    % [kg] mass of ship
leng   = 2.5;               % [m] length of ship
width = 0.6;                % [m] width of ship parts in water
depth = 0.4;                % [m] depth of ship parts in water
Iz = 42.25;                 % [kg*m^2] moment of inertia = 2*50kg*(0.65m)^2 
max_thrust = 136;           % [N] maximum thrust of 1 engine
max_vel = 1.1;              % [m/s] maximum velocity of 1 engine
pos_eng = [-0.65 0.65];     % [m] position of engines on y-coordinate
C_X_hat = -0.007;           % wind coefficient, see page 36
C_Y_hat = -0.016;           % wind coefficient, see page 36
C_N1_hat = -0;              % wind coefficient, see page 36
C_N2_hat = -0.0;            % wind coefficient, see page 36
C_D1 = 1.8;                 % damping coefficient, X-direction, 2*max_thrust/(max_vel^2*width*depth*dens_water);
C_D2 = 1.8;                 % damping coefficient, Y-direction
C_D3 = 1.8;                 % damping coefficient, turn

% sensors
sigma_gps = 2.5*1.2;        % [m] CEP to RMS = x1.2 (see http://en.wikipedia.org/wiki/Circular_error_probable)
update_gps = 1/4;           % [s]
sigma_compass = 4/180*pi;   % [rad]
update_compass = 1/15;      % [s]

% kalman filter variables
% ---------------------------------
phi = [1 0 0 update_gps 0 0;        % 1st order kalman filter
       0 1 0 0 update_gps 0;
       0 0 1 0 0 update_gps;
       0 0 0 1 0 0;
       0 0 0 0 1 0;
       0 0 0 0 0 1];
gamma = 0.01;  % process noise: Q->0: 100% trust in model, Q->inf, 100% trust in measurements

Q = gamma*[update_gps^3/3 0 0 update_gps^2/2 0 0;
           0 update_gps^3/3 0 0 update_gps^2/2 0;
           0 0 update_gps^3/3 0 0 update_gps^2/2;
           update_gps^2/2 0 0 update_gps 0 0;
           0 update_gps^2/2 0 0 update_gps 0;
           0 0 update_gps^2/2 0 0 update_gps];
H = [1,0,0,0,0,0;
     0,1,0,0,0,0;
     0,0,1,0,0,0];                  %observation matrix
R = [sigma_gps^2 0 0;
     0 sigma_gps^2 0;
     0 0 sigma_compass^2];            % measurement uncertainty matrix
P = 1000*eye(6);                    % state estimation covariance matrix
G = [0 0 0;
     0 0 0;
     0 0 0;
     1 0 0;
     0 1 0;
     0 0 1];                        % control matrix
u = [0;0;0];
x_hat = [0;0;0;0;0;0];              % x,y,psi,x_dot,y_dot,psi_dot - all in earth frame

% position control variables
% ---------------------------------
kr=3;
ka=8;
kb=-1.5;





% program
% ---------------------------------
t=0;
X_p = [];
Y_p = [];
N_p = [];
vel_p = [];
figure(1);
t_gps = 100;
t_compass = 100;
gi=1;
goal = pathpoints(gi,:);
stop_sim = 0;

P_p = [];
Err_p = [];

while t < T_sim && stop_sim == 0
    % forces
    % ---------------------------------
    % thrust motor 
    X_motor = thrust1*max_thrust+thrust2*max_thrust;
    Y_motor = 0;
    N_motor = pos_eng(1)*thrust1*max_thrust+pos_eng(2)*thrust2*max_thrust;
    
    % damping terms (depends on speed of boat)
    X_damping = sign(vel(1))*width*depth*0.5*dens_water*vel(1)^2*C_D1;
    Y_damping = sign(vel(2))*leng*depth*0.5*dens_water*vel(2)^2*C_D2;
    N_damping = -sign(vel(3))*leng*depth*pos_eng(1)*dens_water*vel(3)^2*C_D3;
%     X_damping = 0;
%     Y_damping = 0;
%     N_damping = 0;

    % current (is assumed to be zero)
    u_c = 0;
    v_c = 0;
    X_current = 0;
    Y_current = 0;
    N_current = 0;

    % wind
    u_r = v_wind*cos(d_wind-pose(3)) + vel(1) + u_c;         %Nach Buch minus, macht aber keinen Sinn da Kr�fte zunehmen!
    v_r = v_wind*sin(d_wind-pose(3)) + vel(2) + v_c;

    V_wind = sqrt(u_r^2 + v_r^2);

    g_r = atan2(v_r,u_r);
    while g_r > pi
        g_r = g_r-pi;
    end
    while g_r < -pi
        g_r = g_r+pi;
    end
    C_X = C_X_hat*cos(g_r);
    C_Y = C_Y_hat*sin(g_r);
    if g_r > pi/2 && g_r < 3*pi/2
        C_N_hat = C_N2_hat;
    else
        C_N_hat = C_N1_hat;
    end
    C_N = C_N_hat*sin(2*g_r);

    X_wind = 1/2*dens_air*V_wind^2*C_X*leng^2;
    Y_wind = 1/2*dens_air*V_wind^2*C_Y*leng^2;
    N_wind = 1/2*dens_air*V_wind^2*C_N*leng^3;
    
%     X_wind = 0;
%     Y_wind = 0;
%     N_wind = 0;

    % waves

    w = 2*pi/T;
    k = 4*pi^2/(g*T^2);

    chi = pi - (d_waves-pose(3));                   
    w_e = w - k*vel(1)*cos(chi)+k*vel(2)*sin(chi); 
    a = dens_water*g*(1-exp(-k*depth))/k^2;
    b = k*leng/2*cos(chi);
    c = k*width/2*sin(chi);
    s = (k*h/2)*sin(w_e*t);
    eps = h/2*cos(w_e*t);

    if c==0, c=0.001, end
    if b==0, b=0.001, end
    X_waves = 2*a*width*sin(b)*sin(c)/c*s;     %not so nice (c=0 or b=0)
    Y_waves = -2*a*leng*sin(b)*sin(c)/b*s;
    N_waves = a*k*(width^2*sin(b)*(c*cos(c)-sin(c))/c^2 - leng^2*sin(c)*(b*cos(b)-sin(b))/b^2)*eps;   %olis correction term: 1/6
% 
%     X_waves = 0;
%     Y_waves = 0;
%     N_waves = 0;

    X=X_motor+X_current+X_wind+X_waves-X_damping;
    Y=Y_motor+Y_current+Y_wind+Y_waves-Y_damping;
    N=N_motor+N_current+N_wind+N_waves-N_damping;
    
    X_p = [X_p X];
    Y_p = [Y_p Y];
    N_p = [N_p N];
    
    vel_new = vel;
    vel_new(3) = delta_t/Iz*N+vel(3);
    vel_new(2) = 1/(1+vel_new(3)*delta_t^2)*(Y/m*delta_t+vel(2)-delta_t^2*X/m);
    vel_new(1) = delta_t*(X/m+vel_new(2)*vel_new(3))+vel(1);
    vel = vel_new;
    vel_p = [vel_p vel];    
    
    psi = pose(3);
    Rot = [cos(psi) -sin(psi) 0; sin(psi) cos(psi) 0; 0 0 1];
    pose = pose + Rot*vel_new*delta_t;
    
    % measurements
    % ---------------------------------
    
    if t_gps >= update_gps
        gps_length = random('norm',0,sigma_gps);
        gps_orient = random('unif',0,2*pi);
    end
    if t_compass >= update_compass
        compass_orient = random('norm',0,sigma_compass);
        t_compass = 0;
    end
    sens_point = pose + [gps_length*cos(gps_orient); gps_length*sin(gps_orient); compass_orient];
    
    % filtering
    % ---------------------------------
    % GPS
    if t_gps >= update_gps
        % Standard first-order Kalman-Filter with noise
        y = sens_point;

        K = P*H'*(H*P*H' + R)^-1;                       %Compute Kalman gain
        P = (eye(6)-K*H)*P;                             %update state vector error covariance matrix

    
        x_hat = x_hat + K*(y-H*x_hat);                  %Update state estimate

        t_gps = 0;

        % Update time of gps too short and the boat is moving too slow that the
        % calculation of a position prediction is needed.
    
    end
    
%     rot = [cos(x_hat(3)) -sin(x_hat(3)) 0; sin(x_hat(3)) cos(x_hat(3)) 0; 0 0 1];
%     u = rot*[thrust1/200+thrust2/200;0;thrust1/(200*pos_eng(1))+thrust2/(200*pos_eng(2))];    
%

    
    P = phi*P*phi'+Q;                                % prediction step
    x_hat = phi*x_hat+G*u;                       %position prediction step

    P_p = [P_p sqrt(sqrt(P(1,1)^2+P(2,2)^2))];
    Err_p = [Err_p sqrt((x_hat(1)-pose(1))^2+(x_hat(2)-pose(2))^2)];

    
    % position control
    % ---------------------------------

    if pos_control == 1
        roh = sqrt((x_hat(1)-goal(1))^2+(x_hat(2)-goal(2))^2);
        if roh < 2                   % get next goal
            gi=gi+1;
            if gi>length(pathpoints)
                stop_sim = 1;  % if arrived at goal finish simulation
            else
                goal = pathpoints(gi,:);
            end
        end
        alpha = atan2(goal(2)-x_hat(2),goal(1)-x_hat(1)) - x_hat(3);
        beta = 0;       % does not depend on angle - otherwise: x_hat(3)-alpha;
        v = kr*roh*cos(alpha);
        w = ka*alpha+kb*beta;
        thrust1 = v+w*pos_eng(1);
        thrust2 = v+w*pos_eng(2);
        if abs(thrust1) > 1 || abs(thrust2) > 1 
            if abs(thrust1) > abs(thrust2)
                thrust2 = thrust2/abs(thrust1);
                thrust1 = sign(thrust1)*1;
            else
                thrust1 = thrust1/abs(thrust2);
                thrust2 = sign(thrust2)*1;
            end
        end
    end
    
    % plotting
    % ---------------------------------

    if abs(pose(1)) > world_size/2
        world_size = abs(pose(1))*2;
    end
    if abs(pose(2)) > world_size/2
        world_size = abs(pose(2))*2;
    end
    
    psi = pose(3);
    boat = [ones(1,5)*pose(1); ones(1,5)*pose(2)] + ([0,-leng/2, +leng/2, -leng/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(psi) sin(psi); -sin(psi) cos(psi)])';
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
    
    if disp_measurements == 1
        measurement = [ones(1,5)*sens_point(1); ones(1,5)*sens_point(2)] + ([0,-leng/2, +leng/2, -leng/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(sens_point(3)) sin(sens_point(3)); -sin(sens_point(3)) cos(sens_point(3))])';
        filtered = [ones(1,5)*x_hat(1); ones(1,5)*x_hat(2)] + ([0,-leng/2, +leng/2, -leng/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(x_hat(3)) sin(x_hat(3)); -sin(x_hat(3)) cos(x_hat(3))])';

        mu = [x_hat(1),x_hat(2)];
        ellipse=0:pi/30:2*pi; % angles around a circle
        [eigvec,eigval] = eig([P(1,1:2);P(2,1:2)]); % Compute eigen-stuff
        xyellipse = [cos(ellipse'),sin(ellipse')] * sqrt(eigval) * eigvec'; % Transformation
        xellipse = xyellipse(:,1);           
        yellipse = xyellipse(:,2);        
    end
    
    if disp_forces == 1
        if disp_measurements == 1
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g',measurement(2,:),measurement(1,:),'r',filtered(2,:),filtered(1,:),'g',goal(2),goal(1),'o');      %coordinate system is other way round
        else
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g',goal(2),goal(1),'o');      %coordinate system is other way round
        end
    else
        if disp_measurements == 1
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',measurement(2,:),measurement(1,:),'r',filtered(2,:),filtered(1,:),'g',goal(2),goal(1),'o',mu(2)+yellipse,mu(1)+xellipse,'-');      %coordinate system is other way round
        else
            plot(boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',goal(2),goal(1),'o');      %coordinate system is other way round
        end
    end
    axis([-world_size/2 world_size/2 -world_size/2 world_size/2]);

%    title([num2str(pose(1)),', ', num2str(pose(2)), ', ', num2str(pose(3)*180/pi),', Windwinkel zum Boot = ',num2str(g_r*180/pi),'�',', chi = ',num2str(chi/pi*180)]);
    title(['Simulation time = ',num2str(t),' s']);
    drawnow;
    pause(delta_t);

    % time shift
    % ---------------------------------
    
    t_gps = t_gps + delta_t;
    t_compass = t_compass + delta_t;        
    t=t+delta_t;
end
t=linspace(0,t,length(N_p));
figure(2);
plot(t,X_p,'r',t,Y_p,'g',t,N_p,'b');
title('forces');
xlabel('time [s]');
ylabel('forces [N]');

figure(3);
plot(t,vel_p(1,:),'r',t,vel_p(2,:),'g',t,vel_p(3,:),'b');
title('velocities');
xlabel('time [s]');
ylabel('velocities [m/s]');

figure(4);
plot(t,P_p,'g',t,Err_p,'r');
title('Estimation position error and true deviation');
xlabel('time [s]');
ylabel('distance [m]');

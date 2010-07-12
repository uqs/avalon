%
%       Mathematical model of the movement of the ship in the water
%       with influence of wind, current and waves
%
% - Parameters
%
%       (c) by Fabian Jenne
%       see documentation in "Identification and adaptive control applied to
%       ship steering" by Claes G K�llstr�m
%       and "Endbericht Final Avalon Team"

%  simulation variables
%  -------------------------------------------------------------------------
T_sim                   = 1000;
delta_t                 = 0.4;% [s] iteration time step (smallest time step of sensor reading)
local_size              = 3000;
world_size              = 14000;          % [m] size of the part we are looking at
disp_forces             = 1;            % display forces; 0 or 1
disp_measurements       = 0;            % display measurements; 0 or 1
rcflags_emergency_stop  = 0;
ais_sim                 = 0;            % 1: simulate other boats and detect them with AIS sensor          

r_earth                 = 6371000.785;

t = 0;
n = 1;                              % for AIS, for Plots
i = 1;                              % for AIS
j = 1;
X_drag  = [];
Y_drag  = [];
X_p     = [];
Y_p     = [];
N_p     = [];
pose3_p = [];
vel_p   = [];
alpha_rudder_p = [];

% AIS Boat Parameter
% -------------------------------------------------------------------------
ais_length  = 2;                 % [m] length of a "AIS" boat
ais_width   = 1;                  % [m] width of a "AIS" boat
ais_range   = 50;               % [m] AIS Sensor-range to detect AIS boats

% unchangeable variables
% -------------------------------------------------------------------------
dens_air    = 1.184;                % [kg/m^3] density of air
dens_water  = 1025;              % [kg/m^3] density of water
g           = 9.81;                        % [m/s^2]

% changeable variables
% -------------------------------------------------------------------------

% initial kinematic variables
vel         = [0;0;0];                  % u [m/s], v [m/s], r [rad/s]
pose        = [0;0;0];                 % x [m], y [m], psi [rad] start pose

% boat variables
m           = 450;                    % [kg] mass of the ship
length      = 3.95;                   % [m] length of ship
width       = 1.2;                    % [m] width of ship parts in water
depth       = 0.2;                    % [m] depth of ship parts in water
I           = [635;2427;120]; % I(3)=150;             % [kg*m^2] moment of inertia

A_sail      = 8.4;                   % [m^2] Area of the sail
A_hull      = [0.35;4;2.0];           % [m^2] Area for hull drag
C_d         = [0.08;4;1.3];         % C_d(2)=1.3 ....damping coefficient, for hull drag
C_hat       = [-0.007;-0.016;0;-0.0025];   % wind coefficient, see page 36
% C_hat       = [-0.013;-0.048;-0.0015;-0.0075];
A_rudder    = 0.085;               % [m²] Area of one rudder (=0.17*0.5)


% environment variabels
% -------------------------------------------------------------------------

% wind, sail, rudder given in gui

% current
v_current       = 0.5;                  % [m/s] current velocity
d_current       = deg2rad(1);         % [rad] current direction, 180�: current from behind the ship

% waves
d_waves         = deg2rad(1);           % [rad] wave direction (180�: waves coming from behind
h               = 0.5;                        % [m] wave height
T               = 50;                         % [s] wave period, MUST BE LONGER, THEN THE SHIP LENGTH -> unstable otherwise

% sensors
sigma_gps       = 2.5*1.2;        % [m] CEP to RMS = x1.2 (see http://en.wikipedia.org/wiki/Circular_error_probable)
update_gps      = 1/4;           % [s]
sigma_compass   = 4/180*pi;   % [rad]
update_compass  = 1/15;      % [s]
gps_length      = 0;             % initial. for first step
gps_orient      = 0;

% particle filter variables
% -------------------------------------------------------------------------

n_particles     = 150;                  % number of particles
filter_size     = 20;                   % [m] diameter of circle in which particles are spread around ship
x_hat           = [0,0,0];



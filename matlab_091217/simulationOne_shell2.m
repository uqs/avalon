%
%            Mathematical model of the movement of the ship in the water
%            with influence of wind, current and waves
%
% - Simulation
%
%           (c) by Fabian Jenne
%           see documentation in "Identification and adaptive control applied to
%           ship steering" by Claes G K�llstr�m
%           and "Endbericht Final Avalon Team"

%% set path for the DDX conection
clear all
clc

addpath '/usr/local/lib/matlab/ddxmatlab'
addpath '.'
%% init variables
double d_wind;
double v_wind;
v_wind=15;
d_wind=130*pi/180;
avalon = ddx_init();
2;
parameter_shell2;

sail_factor = 0.1;

%% Run Simulation with the Position Update Function PoseStep.m
alpha_rudder_r_des      = 0;
alpha_rudder_r          = 0;
alpha_rudder_l          = 0;
aoa_sail                = 0;
timer                   = 0;
des_heading             = 0;
p1_0                    = 0;%1.111949403453934e+06;  % lat,  y-axis
p2_0                    = 0;%-4.380225573914934e+06;  % long, x-axis
save_length             = 180; % the parameters are stored every save_length second
ind=1;
imu.gyro.z = 0;
TT=0;
sim_time=delta_t/2;
data_size=round(save_length/delta_t)+1;
delta_head=zeros(1,data_size);
des_head=zeros(1,data_size);
heading=zeros(1,data_size);
delta_rudder=zeros(1,data_size);
rudder_pos=zeros(1,data_size);
rudder_pos_des=zeros(1,data_size);%     pose3_p(n)                             = pose(3);
a_tot = zeros(1,7);
%     destData.longitude
% 
%     traj(n,:)                           = [(pose(1)-p1_0),(pose(2)-p2_0),pose(3)];
tic
alpha_rudder_hist=zeros(1,data_size);
pose3_p=zeros(1,data_size);
traj=zeros(data_size,3);
% other boats
% num_boats = str2double(get(handles.num_boats,'String'));
% boat_x                  = ones(1,num_boats)*p1_0-local_size/2; % [m]
% boat_y                  = ones(1,num_boats)*p2_0-local_size/2;  % [m]
% boat_heading            = deg2rad(34);%90*rand(1,num_boats)); % in rad
% boat_speed              =(30 + (50-30)*rand(1,num_boats))*0.5144; % [m/s]
% wypx                    = []
rudder_position = [];
wind_d_hist = [];
cl_wind_d_hist = [];
N_sail_hist = [];
d_water_hist = [];
while (t < T_sim)
    %% read DDX Store vari5ables
a(13)=toc;
%    vel
a;
%         tic
    if(mod(round(t*1000)/1000,60)==0)
%        d_wind=reminderRad(d_wind+10*pi/180);
       sprintf('wind angle increased to %f degrees',d_wind*180/pi)
%         a()=toc
	TT(end+1)=toc;
    toc
        tic
%         if(TT(end)>65)
%             delta_t=delta_t+0.01;
%             sprintf('delta_t increase to %f seconds', delta_t);
%         end
	t_elaps_old=0;
        %a_tot(end+1,:)=a;
        t
    end
a(1)=toc;

t_elaps_s=toc;
while (t_elaps_s-t_elaps_old)<(delta_t-sim_time-0.01)
% while t_elaps<delta_t
t_elaps_s=toc;
end
    [rudder, sail, flags, rcflags, destData, wypData, joystick, imu_clean, wind, cleanwind] = ddx_read_shell2( avalon );
wind_d_hist(end+1)=cleanwind.global_direction_real;
a(2)=toc;
    vel(1,1)            = imu_clean.velocity.x*0.5144; % knots into [m/s]
    vel(2,1)            = -imu_clean.velocity.y*0.5144;% [m/s]
    vel(3,1)            = imu.gyro.z*pi/180;
    
%     torque_des          = rudder.torque_des;
    %des_heading         = pi/180*desiredheading.heading;
    
    %pose                = ll2xy(imu.position.longitude,imu.position.latitude);
    %pose(3)             = reminderRad(imu.attitude.yaw*pi/180);
    
    aoa_sail_des        = reminderRad(pi/180*(sail.degrees));          % sailTarget
    alpha_rudder_l_des  = reminderRad(pi/180*(rudder.degrees_left));  % rudderTarget
    alpha_rudder_r_des  = reminderRad(pi/180*(rudder.degrees_right));       
    
    aoa_sail            = reminderRad(sailUpdate(aoa_sail_des, aoa_sail, delta_t, pi/180*(15))); 
    alpha_rudder_l      = rudderUpdate(alpha_rudder_l_des, alpha_rudder_l, delta_t, pi/180*(30));  %35 for delta_t = 0.2
    alpha_rudder_r      = rudderUpdate(alpha_rudder_r_des, alpha_rudder_r, delta_t, pi/180*(30));
    rudder_position(end+1)=alpha_rudder_r;
%     alpha_rudder_hist(n)      = rad2deg(alpha_rudder_r);
%     rudder_pos_des(n)=(rudder.degrees_left);
%     des_head(n)=des_heading;
%     heading(n)=pose(3);
%     delta_head(n)=rad2deg(des_heading-pose(3));
    
    flags_state                         = flags.state;
    rcflags_sailorstate_requested       = rcflags.sailorstate_requested;
    
%     i=1;
%     while destData.Data(i).latitude~=0
%         dest_x(i)=destData.Data(i).latitude-p1_0;
%         dest_y(i)=destData.Data(i).longitude-p2_0;
%         i=i+1;
%     end
    wp_x=0;
    wp_y=0;
    k=1;
    while (wypData.Data(k).passed == 1)
        k=k+1;
    end
% %     (wypData.Data(1:k).wyp_type)
% %     k
% %     for i=1:100
% %         wp_x_temp(i)=wypData.Data(i).latitude;
% %         wp_y_temp(i)=wypData.Data(i).longitude;
% %         wp_head(i)=wypData.Data(i).heading;
% %     end
i=1;
clear wp_x_loc wp_y_loc wp_x_glo wp_y_glo wp_head
% wypData.Data(i).wyp_type;
% 
wp_x_glo(i)=wypData.Data(i).x;
        wp_y_glo(i)=wypData.Data(i).y;
        wp_head(i)=wypData.Data(i).heading;
        i=2;
while (wypData.Data(i-1).wyp_type ~= 1)
        wp_x_glo(i)=wypData.Data(i).x;
        wp_y_glo(i)=wypData.Data(i).y;
        wp_head(i)=wypData.Data(i).heading;
        i=i+1;
        if i>100
            break
        end
end
% 
%     wp_x_loc=wp_x_glo-p1_0;
%     wp_y_loc=wp_y_glo-p2_0;
% %     wp_head;
% % %     wp_x=[wp_x_temp(1);nonzeros(wp_x_temp(2:end))]-p1_0;
% % %     wp_y=[wp_y_temp(1);nonzeros(wp_y_temp(2:end))]-p2_0;
% % %     wp_head=nonzeros(wp_head);
%     destinationx                        = destData.latitude-p1_0;
%     destinationy                        = destData.longitude-p2_0;
%     

    %% control steps for position update
    if t == 0
        rcflags_sailorstate_requested     = 3;          % = AV_FLAGS_ST_NORMALSAILING
        rcflags.man_in_charge               = 1; % = AV_FLAGS_MIC_SAILOR
        rcflags.autonom_navigation          = 1;
%         pose(1,1)                         =  1.111949403453934e+06;
%         pose(2,1)                         = -4.380225573914934e+06;
        vel(1,1)                          = 5;
	vel(2,1)			  = 0;
	vel(3,1)			  = 0.1;
        i=1;
	t_elaps_old=0;
    imu.position.latitude               = destData.Data(2).latitude;
    imu.position.longitude              = destData.Data(2).longitude;
%     while destData.Data(i).latitude ~=0
%             X=ll2xy(destData.Data(i).longitude, destData.Data(i).latitude);
%             dest_x_t(i) = X(1);
%             dest_y_t(i) = X(2);
%             i=i+1;
%     end
%             dest_x=dest_x_t-p1_0;
%             dest_y=dest_y_t-p2_0;

%         for l=1:i-2
%             dist_desti(l)=sqrt((dest_x(l)-dest_x(l+1))^2+(dest_y(l)-dest_y(l+1))^2);
%         end
%         save dist dist_desti
%         dist_boat                         =[sqrt((pose(1)-boat_x).^2+(pose(2)-boat_y).^2) zeros(1,5-num_boats)];
%         dist_min                          = min(sqrt((pose(1)-boat_x).^2+(pose(2)-boat_y).^2));
    end
for i=1:20
    dest_y(i) = r_earth*cos(destData.latitude*pi/180)*pi/180*(destData.Data(i).longitude-destData.longitude);
    dest_x(i) = r_earth*pi/180*(destData.Data(i).latitude-destData.latitude);
end
pose(2) = r_earth*cos(destData.latitude*pi/180)*pi/180*(imu.position.longitude-destData.longitude);
pose(1) = r_earth*pi/180*(imu.position.latitude-destData.latitude);
a(3)=toc;
%vel
%pose
%aoa_sail
%d_wind
    [pose, vel, X, Y, N, X_waves, Y_waves, N_waves, X_sail, Y_sail, N_sail, N_rudder, N_damping, V_wind, g_r, d_water] = PoseStep_shell(t, delta_t, pose, vel, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, v_current, d_current, v_wind, d_wind, d_waves, T, h, depth, length, width, sail_factor);
    %[pose, vel, X, Y, N, N_rudder, N_damping] = PoseStep_only_rudder(t, delta_t, pose, vel, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, T, h, depth, length, width, sail_factor);
%     ax_lim=[-local_size/2+pose(2)-p2_0 local_size/2+pose(2)-p2_0
%     -local_size/2+pose(1)-p1_0 local_size/2+pose(1)-p1_0];
    vel;
%     N
%     X
%     Yimu.attitude.yaw=-remainder(reply.getOriEuler().m_yaw,360.0);    // yaw, Avalon-convention
a(10)=toc;    
    
%     num_boats_temp = str2double(get(handles.num_boats,'String'));
%     if num_boats_temp>num_boats
%         boat_x(num_boats+1:num_boats_temp) %     boat_x      = boat_x + boat_speed.*cos(boat_heading)*delta_t;
%     boat_y      = boat_y + boat_speed.*sin(boat_heading)*delta_t;
%        =ones(1,num_boats_temp-num_boats)*pose(1)+local_size/2; % [m]
%         boat_y(num_boats+1:num_boats_temp)        =ones(1,num_boats_temp-num_boats)*pose(2)-local_size/2;  % [m]
%         boat_heading(num_boats+1:num_boats_temp)  =deg2rad(120);%90*rand(1,num_boats_temp-num_boats)); % in rad
%         boat_speed(num_boats+1:num_boats_temp)    =17;%(30 + (50-30)*rand(1,num_boats_temp-num_boats))*0.5144; % [m/s]   
%     end
%     if num_boats_temp<num_boats
%         boat_x          =boat_x(1:num_boats_temp); % [m]
%         boat_y          =boat_y(1:num_boats_temp);  % [m]
%         boat_heading    =boat_heading(1:num_boats_temp); % in rad
%         boat_speed      =boat_speed(1:num_boats_temp); % [m/s]   
%     end
%     num_boats=num_boats_temp;
%     for i=1:num_boats
%         if boat_x(i)-p1_0<ax_lim(3)
%             boat_heading(i)=deg2rad(180*rand-90);
%         end
%         if boat_x(i)-p1_0>ax_lim(4)
%             boat_heading(i)=deg2rad(-180*rand-90);
%         end
%         if boat_y(i)-p2_0<ax_lim(1)
%             boat_heading(i)=deg2rad(180*rand);
%         end
%         if boat_y(i)-p2_0>ax_lim(2)
%             boat_heading(i)=deg2rad(-180*rand);
%         end
%     end
    
    
%     pose3_p(n)                             = pose(3);
%     
%
%      traj(n,:)                           = [(pose(1)-p1_0),(pose(2)-p2_0),pose(3)];

%     boat_x      = boat_x + boat_spe6ed.*cos(boat_heading)*delta_t;
%     boat_y      = boat_y + boat_speed.*sin(boat_heading)*delta_t;
    pose_plot=point2boat([pose(2) pose(1) pose(3)],50);
%     destpoint_i                         = [(destinationx-p1_0) (destinationy-p2_0)];
%     ax_lim=[-local_size/2+pose(2)-p2_0 local_size/2+pose(2)-p2_0 -local_size/2+pose(1)-p1_0 local_size/2+pose(1)-p1_0];
%     wind_p    = [ones(1,5)*ax_lim(3)+local_size/20; ones(1,5)*ax_lim(1)+local_size/20] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
%     


%     try
%     wp_x_loc=nonzeros(wp_x_glo)+dest_x(destData.destNr+1);
%     wp_y_loc=nonzeros(wp_y_glo)+dest_y(destData.destNr+1);
try
    wp_x_loc=wp_x_glo+dest_x(destData.destNr+1);
    wp_y_loc=wp_y_glo+dest_y(destData.destNr+1);
    plot(pose_plot(:,1),pose_plot(:,2),'b',pose(2),pose(1),'b',wp_y_loc,wp_x_loc,'ko-',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', dest_y(destData.destNr+1), dest_x(destData.destNr+1),'sr')
%     axis(ax_lim)
%     axis([-2500 2500 -3000 1500])
axis([-6500 1500 -4000 5000])
%     axis('equal')
    drawnow;
    end
    %% set speed (along current orientation) in the gui
    
%     avalon_speed_ms = sqrt(vel(1,1)^2 + vel(2,1)^2)    % [m/s]
%     avalon_speed_kn = avalon_speed_ms/0.5144;       % [kn]

    
    
    %% write variables to DDX Store
    
%     [imu.position.longitude, imu.position.latitude] = xy2ll(pose);
%     if (~isempty(boat_x))
%         dist_boat(n,:)=[sqrt((pose(1)-boat_x).^2+(pose(2)-boat_y).^2) zeros(1,5-num_boats)];
%         
%         dist_min(n)=min(dist_boat(n,find(dist_boat(n,:))));
%         av_speed=sqrt(vel(1,1)^2+vel(2,1)^2);
%         t_min=((pose(2)-boat_y).*(sin(boat_heading).*boat_speed-sin(pose(3)+atan2(vel(2),vel(1)))*av_speed)+(pose(1)-boat_x).*(cos(boat_heading).*boat_speed-cos(pose(3)+atan2(vel(2),vel(1)))*av_speed))./((boat_speed.*sin(boat_heading)-av_speed*sin(pose(3)+atan2(vel(2),vel(1))))^2+(boat_speed.*cos(boat_heading)-av_speed*cos(pose(3)+atan2(vel(2),vel(1))))^2);
%         t_coll=t+t_min;
%         curr_min_dist=sqrt((pose(2)+sin(pose(3)+atan2(vel(2),vel(1)))*av_speed.*t_min-boat_y-sin(boat_heading).*boat_speed*t_min).^2+(pose(1)+cos(pose(3)+atan2(vel(2),vel(1)))*av_speed.*t_min-boat_x-cos(boat_heading).*boat_speed.*t_min).^2);
%         if t_min<0
%             t_min=NaN;
%             curr_min_dist=NaN;
%         end
%         if(round(t)==20)
%         22;
% %         pause%     rcflags.emergency_stop              = rcflags_emergency_stop;
%     rcflags.motion_stop                 = 0;
%     rcflags.joystick_timeout            = 0;
%     
%     if flags_state == rcflags_sailorstate_requested;
%         rcflags_sailorstate_requested   = 0;
%     end
%     
%     rcflags.sailorstate_requested       = rcflags_sailorstate_requested;

%         end
%     else
%         dist_boat(n,:)=zeros(1,5);
%         dist_min(n)= NaN;
%         t_min=NaN;
%         curr_min_dist=NaN;
%     end

    imu.position.latitude               = destData.latitude + 180/(r_earth*pi)*pose(1);
    imu.position.longitude              = destData.longitude + 180/(r_earth*pi*cos(destData.latitude*pi/180))*pose(2);
    imu.position.latitude;
    imu.position.longitude;

    imu.attitude.yaw                                = 180/pi*(pose(3));
    
    imu.velocity.x                      = vel(1,1)/0.5144;
    imu.velocity.y                      = -vel(2,1)/0.5144;
    imu.gyro.z                          = 180/pi*(vel(3,1));
    
    
    sailstate.degrees_sail              = 180/pi*(reminderRad( aoa_sail ));
    rudderstateleft.degrees_rudder      = 180/pi*(reminderRad(alpha_rudder_l));
    rudderstateright.degrees_rudder     = 180/pi*(reminderRad(alpha_rudder_r));

    
    V_wind_x=(v_wind*cos(d_wind-pose(3))+vel(1));
V_wind_y=(v_wind*sin(d_wind-pose(3))+vel(2));
V_wind=sqrt((V_wind_x)^2+(V_wind_y)^2);
noise=10*randn;
    wind.speed                          = V_wind/0.5144;                 % v_wind                                        % [kn]
    wind.direction                      = 180/pi*(atan2(V_wind_y,V_wind_x)-aoa_sail) + noise;%rad2deg(reminderRad(d_wind - pose(3) - aoa_sail));   % rad2deg(g_r - aoa_sail -pi)the windsensor is mounted on the mast, so the rot position has to be concidered
    wind.voltage                        = 12;                                           % randomly set to 12 Volt
    wind.temperature                    = 20;                                           % constant temp of 20°
    wind.uptodate                       = 2; % wind Temp/Voltage up to date? -> 0:no,  else:yes
    
%     wind.direction*pi/180;
    
%     for i=1:num_boats
%         [boat_long boat_lat]                = xy2ll([boat_x(i);boat_y(i)]);
% 
%         aisData.Ship(i).mmsi                      = 111111*i;
%         aisData.Ship(i).navigational_status       = 0;
%         aisData.Ship(i).rate_of_turn              = 0;
%         aisData.Ship(i).speed_over_ground         = round(boat_speed(i)/0.5144444);
%         aisData.Ship(i).position_accuracy         = 1;
%         aisData.Ship(i).longitude                 = boat_long;
%         aisData.Ship(i).latitude                  = boat_lat;
%         aisData.Ship(i).course_over_ground        = boat_heading(i)*180/pi;
%         aisData.Ship(i).heading                   = boat_heading(i)*180/pi;
%         aisData.Ship(i).destination(1)            = 0;
%         aisData.Ship(i).time_of_arrival           = 0;
%     end
%     
%     aisData.number_of_ships             = num_boats;
    
    %variables for the rcflags, if remotecontroll.cpp is not used !
    %----------------------------------------------------------------------------------------------------
%     rcflags.emergency_stop              = rcflags_emergency_stop;
%     rcflags.motion_stop                 = 0;
%     rcflags.joystick_timeout            = 0;
%     
    if flags_state == rcflags_sailorstate_requested;
        rcflags_sailorstate_requested   = 0;
    end
%     
    rcflags.sailorstate_requested       = rcflags_sailorstate_requested;
%     rcflags.man_in_charge               = 1; % = AV_FLAGS_MIC_SAILOR
%     rcflags.autonom_navigation          = 1;
    a(11)=toc;
    aisData=0;
t_elaps=toc;
sim_time=t_elaps-t_elaps_old;
wind_d_hist(end+1)=d_wind*180/pi + noise;
cl_wind_d_hist(end+1)=cleanwind.global_direction_real;
N_sail_hist(end+1)=N_sail;
d_water_hist(end+1)=d_water;
while (t_elaps-t_elaps_old)<delta_t
% while t_elaps<delta_t
t_elaps=toc;
end
t_elaps_old=t_elaps;
    ddx_write_shell( avalon, wind, rudder, rcflags, sailstate, rudderstateright, rudderstateleft, imu, aisData)
% tic
	t = t + delta_t;
%     if(n==data_size)
%         TT
%         toc
%         file = ['Simulation_data/data', num2str(ind)];
%         save(file, 'traj', 'dest_x', 'dest_y', 'wp_x_loc', 'wp_y_loc','destinationx','destinationy','k','delta_head','des_head','heading','delta_t' )
%         save('Simulation_data/ind', 'ind');
%         ind=ind+1;
%         traj=zeros(data_size,3);
%         n=0;
%         tic
%         t
%     end
%    dist_dest=sqrt((pose(1)-dest_x(end)-p1_0)^2+(pose(2)-dest_y(end)-p2_0)^2);
 %   if (dist_dest<200)
  %      dist_dest
   %     break;
    %end
    n=n+1;
    a(12)=toc;
%     save data;
%     save data traj;
end

ddx_close( avalon );

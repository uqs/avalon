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

addpath '/usr/local/lib/matlab/ddxmatlab'
addpath '.'

%% init variables
double d_wind;
double v_wind;
v_wind=15;
set(handles.wind_speed,'String',num2str(v_wind));
d_wind=-pi/3;
set(handles.wind_angle,'String',num2str(rad2deg(d_wind)));
avalon = ddx_init();

parameter;

sail_factor = 0.1;

%% Run Simulation with the Position Update Function PoseStep.m
alpha_rudder_r_des      = 0;
alpha_rudder_r          = 0;
alpha_rudder_l          = 0;
aoa_sail                = 0;
timer                   = 0;
des_heading             = 0;
p1_0                    = 1.111949403453934e+06;  % lat,  y-axis
p2_0                    = -4.380225573914934e+06;  % long, x-axis
delta_head=[];
delta_rudder=[];
rudder_pos=[];
rudder_pos_des=[];
% other boats
num_boats = str2double(get(handles.num_boats,'String'));
boat_x                  = ones(1,num_boats)*p1_0-local_size/2; % [m]
boat_y                  = ones(1,num_boats)*p2_0-local_size/2;  % [m]
boat_heading            = deg2rad(34);%90*rand(1,num_boats)); % in rad
boat_speed              =(30 + (50-30)*rand(1,num_boats))*0.5144; % [m/s]

% wypx                    = []
while (t < T_sim && ~Stop && ~rcflags_emergency_stop)
    %% read DDX Store vari5ables
%     boat_heading*180/pi
    [joystick, rudder, sail, flags, rcflags, desiredheading, imu, cleanimu, destStruct, destData, aisData, wypStruct, wypData] = ddx_read( avalon );
    if (mod(round(t),30)==29)
%         pause;
2;
    end
    
    vel(1,1)            = cleanimu.velocity.x*0.5144; % knots into [m/s]
    vel(2,1)            = -cleanimu.velocity.y*0.5144;% [m/s]
    vel(3,1)            = deg2rad(imu.gyro.z);
    set(handles.heading_speed   ,'String' , num2str(rad2deg(vel(3,1))));
    
    torque_des          = rudder.torque_des;
    
    [pose]              = ll2xy(imu.position.longitude,imu.position.latitude);
    
    pose(3)             = deg2rad(imu.attitude.yaw);  pose(3) = reminderRad(pose(3));
    
    joy_but1            = joystick.buttons(1);          % shooting button
    joy_but2            = joystick.buttons(2);          % thumb button
    joy_but3            = joystick.buttons(3);          % button nr 3
    joy_but4            = joystick.buttons(4);          % interfere with sail and rudder
    joy_but5            = joystick.buttons(5);          % des heading + / - 45°
    joy_but6            = joystick.buttons(6);
    joy_but7            = joystick.buttons(7);
    joy_but8            = joystick.buttons(8);
    joy_but9            = joystick.buttons(9);          % emergency stop
    joy_axes1           = double(joystick.axes(1));     % left - right
    joy_axes2           = double(joystick.axes(2));     % front - back
    joy_axes3           = double(joystick.axes(3));     % turn around z-axis
    joy_axes4           = double(joystick.axes(4));     % small additional turning stick
    
    des_heading         = deg2rad(desiredheading.heading);
    delta_head=[delta_head rad2deg(des_heading-pose(3))];
    
    aoa_sail_des        = deg2rad(sail.degrees);           aoa_sail_des        = reminderRad(aoa_sail_des);        % sailTarget
    alpha_rudder_l_des  = deg2rad(-rudder.degrees_left);    alpha_rudder_l_des  = reminderRad(alpha_rudder_l_des);    % rudderTarget
    alpha_rudder_r_des  = deg2rad(-rudder.degrees_right);   alpha_rudder_r_des  = reminderRad(alpha_rudder_r_des);
    
    
    
    aoa_sail            = sailUpdate(aoa_sail_des, aoa_sail, delta_t, deg2rad(15));   aoa_sail        = reminderRad(aoa_sail);
    alpha_rudder_l      = alpha_rudder_l_des;%rudderUpdate(alpha_rudder_l_des, alpha_rudder_l, delta_t, deg2rad(30));  %35 for delta_t = 0.2
    alpha_rudder_r      = alpha_rudder_r_des;%rudderUpdate(alpha_rudder_r_des, alpha_rudder_r, delta_t, deg2rad(30));
    alpha_rudder_p      = [alpha_rudder_p alpha_rudder_r];
    delta_rudder(end+1)=rad2deg(alpha_rudder_r-alpha_rudder_r_des)*1e+03;
    rudder_pos(end+1)=rad2deg(alpha_rudder_r);
    rudder_pos_des(end+1)=(rudder.degrees_left);
    flags_state                         = flags.state;
    rcflags_sailorstate_requested       = rcflags.sailorstate_requested;
    
    for i=1:10
        dest_x(i)=destData.Data(i).latitude-p1_0;
        dest_y(i)=destData.Data(i).longitude-p2_0;
        i=i+1;
    end
    wp_x=0;
    wp_y=0;
    k=1;
    while (wypData.Data(k).passed == 1)
        k=k+1;
    end
%     (wypData.Data(1:k).wyp_type)
%     k
%     for i=1:100
%         wp_x_temp(i)=wypData.Data(i).latitude;
%         wp_y_temp(i)=wypData.Data(i).longitude;
%         wp_head(i)=wypData.Data(i).heading;
%     end
i=1;
clear wp_x_loc wp_y_loc wp_x_glo wp_y_glo wp_head
% wypData.Data(i).wyp_type;
% k;
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

%     wp_x_loc=wp_x_glo-p1_0;
%     wp_y_loc=wp_y_glo-p2_0;
%     wp_head;
%     wp_x=[wp_x_temp(1);nonzeros(wp_x_temp(2:end))]-p1_0;
%     wp_y=[wp_y_temp(1);nonzeros(wp_y_temp(2:end))]-p2_0;
%     wp_head=nonzeros(wp_head);
%     destinationx                        = destData.latitude;
%     destinationy                        = destData.longitude;
    
    
    %% set sail state pop-up menu in GUI
    flags_state_string = {'IDLE     ';'DOCK     ';'NORMAL   ';'TACK     ';'JIBE     ';'UPWIND   ';'DOWNWIND ';'MAXENERGY';'HEADING C'};
    set(handles.SailStatePopupmenu,'String',flags_state_string(flags_state));
%     [flags_state_string(flags_state) desiredheading.heading];
    %% control parameters with joystick
    
    joy_control = get(handles.manInChargeRadiobutton,'Value');
    if joy_control == 1
        % ----------------------------------------------------------- wind speed / direction by joystick
        if joy_but2 == 1;
            v_wind      = v_wind - (joy_axes2/32767);
            set(handles.wind_speed,'String',num2str(v_wind));
            
            d_wind      = d_wind + (joy_axes1/32767)*deg2rad(10);
            set(handles.wind_angle,'String',num2str(rad2deg(d_wind)));
            
            d_waves     = d_wind - deg2rad(1);
            d_current   = d_wind - deg2rad(1);
            
            if d_waves == deg2rad(180);
                d_waves = d_waves - deg2rad(1);
            end
        end
        
        % ---------------------------------------------------------- desired heading by joystick
        if joy_but6 == 1;
            des_heading = deg2rad(25);
        end
        
        if joy_but5 == 1;
            des_heading = deg2rad(-80);
        end
        
        if joy_but3 == 1;
            des_heading = deg2rad(-135);
        end
        
        if joy_but4 == 1;
            des_heading = deg2rad(135);
        end
        % if joy_but3 == 1;
        %     des_heading = des_heading + (joy_axes1/32767)*deg2rad(10);
        % end
        
        %---------------------------------------------------------- emergency button by joystick
        rcflags_emergency_stop = joy_but9;
        set(handles.emergencyStopRadiobutton,'Value',rcflags_emergency_stop)
        
    else
        v_wind          = str2double(get(handles.wind_speed,'String'));
        d_wind          = deg2rad(str2double(get(handles.wind_angle,'String')));
        d_wind          = reminderRad(d_wind);
        d_waves         = d_wind - deg2rad(5);
        d_current       = d_wind - deg2rad(5);
    end
    %---------------------------------------------------------------------------------------------------------
    
    if joy_control == 1 && joy_but7 == 1
        
        % ---------------------------------------------------------- rudder angle by joystick
        if joy_but1 == 1;
            alpha_rudder_r = (joy_axes3/32767)*deg2rad(45);
            set(handles.rudder_angle,'String',num2str(rad2deg(alpha_rudder_r)));
            alpha_rudder_l = alpha_rudder_r;
            
            d_rudderValue = rad2deg(alpha_rudder_r);
            % set slider for rudder angle
            if (isempty(d_rudderValue) || d_rudderValue < -50 || d_rudderValue > 50)
%                 set(handles.slider_rudderangle,'Value',0);
%                 set(handles.slider_rudderangle,'String','0');
            else
%                 set(handles.slider_rudderangle,'Value',d_rudderValue);
            end
            
            set(handles.rudder_angle_left,'String',num2str(rad2deg(alpha_rudder_l)));
            d_rudderValue_l = rad2deg(alpha_rudder_l);
            % set slider for rudder angle
            if (isempty(d_rudderValue_l) || d_rudderValue_l < -50 || d_rudderValue_l > 50)
%                 set(handles.slider_rudderangleLeft,'Value',0);
%                 set(handles.slider_rudderangleLeft,'String','0');
            else
%                 set(handles.slider_rudderangleLeft,'Value',d_rudderValue_l);
            end
        end
        alpha_rudder_l = alpha_rudder_r;
        
        % --------------------------------------------------------- sail angle by joystick
        aoa_sail = -joy_axes4/32767*pi;
%         set(handles.sail_angle,'String',num2str(rad2deg(aoa_sail)));
        d_sailValue=rad2deg(aoa_sail);
        % set slider for sail angle
        if (isempty(d_sailValue) || d_sailValue <= -180 || d_sailValue >= 180)
%             set(handles.slider_sailangle,'Value',0);
%             set(handles.slider_sailangle,'String','0');
        else
%             set(handles.slider_sailangle,'Value',d_sailValue);
        end
    else
        
        % ---------------------------------------------------------- set rudder
%         set(handles.rudder_angle,'String',num2str(rad2deg(alpha_rudder_r)));
        d_rudderValue = rad2deg(alpha_rudder_r);
        % set slider for rudder angle
%         if (isempty(d_rudderValue) || d_rudderValue < -50 || d_rudderValue > 50)
%             set(handles.slider_rudderangle,'Value',0);
%             set(handles.slider_rudderangle,'String','0');
%         else
%             set(handles.slider_rudderangle,'Value',d_rudderValue);
%         end
        
%         set(handles.rudder_angle_left,'String',num2str(rad2deg(alpha_rudder_l)));
        d_rudderValue_l = rad2deg(alpha_rudder_l);
        % set slider for rudder angle
%         if (isempty(d_rudderValue_l) || d_rudderValue_l < -50 || d_rudderValue_l > 50)
% %             set(handles.slider_rudderangleLeft,'Value',0);
% %             set(handles.slider_rudderangleLeft,'String','0');
%         else
% %             set(handles.slider_rudderangleLeft,'Value',d_rudderValue_l);
%         end
        
        % ---------------------------------------------------------- set sail
%         set(handles.sail_angle,'String',num2str(rad2deg(aoa_sail)));
        d_sailValue=rad2deg(aoa_sail);
        % set slider for sail angle
        if (isempty(d_sailValue) || d_sailValue < -185 || d_sailValue > 185)
%             set(handles.slider_sailangle,'Value',0);
%             set(handles.slider_sailangle,'String','0');
        else
%             set(handles.slider_sailangle,'Value',d_sailValue);
        end
        
        
    end
    
    %% update rudder_angle
% %    start_search = atan2(vel(3,1)*1.7,vel(1,1));  % = d_water
%     speed = sqrt( (vel(1,1)^2) + (vel(2,1)^2) );
%     rot = [0;0;vel(3,1)];
%     dist_to_rudder = [-1.7; 0; 0];
%     vel_app_rudder = vel + cross(rot,dist_to_rudder);
%     v_r_tot = sqrt((vel_app_rudder(1))^2 + (vel_app_rudder(2))^2);% + (vel(3,1)*1.7)^2);
%     
%     d_water_app = atan2(vel_app_rudder(2)*0.01,vel_app_rudder(1));%
%     d_water_app = reminderRad(d_water_app);
%     start_search = d_water_app;
%     if speed < 0.3  % [m/s]
%         alpha_rudder_r = deg2rad(0);
%     elseif speed >= 0.3 && speed <= 0.8
%         torque_des = 0.1*torque_des;
%         [under_limit, upper_limit] = f0_45_min_max(torque_des, vel, pose, d_wind, v_wind, aoa_sail);
%         if upper_limit > deg2rad(45); upper_limit = deg2rad(45); end
%         if under_limit < deg2rad(-45); under_limit = deg2rad(-45); end
%         
%         str=['a_r:' num2str(alpha_rudder_r) ' N_Des:' num2str(torque_des) ' velx:' num2str(vel(1,1)) ' vely:' num2str(vel(2,1)) ' velz:' num2str(vel(3,1)) ' d_wind:' num2str(d_wind) ' v_wind:' num2str(v_wind) ' a_sail ' num2str(aoa_sail) ' under_li ' num2str(under_limit) ' upper_li ' num2str(upper_limit)];
%         disp(str)
% %         z = fzero(@(alpha_rudder_r) f0_45(alpha_rudder_r, torque_des, vel, pose, d_wind, v_wind, aoa_sail),[under_limit upper_limit]);        
%         z = fzero(@(alpha_rudder_r) f0_45(alpha_rudder_r, torque_des, vel, pose, d_wind, v_wind, aoa_sail),start_search);
% %         z = deg2rad(-10);
%         alpha_rudder_r_des = z;
%     elseif speed > 0.8
%         torque_des = 0.8*torque_des;
%         [under_limit, upper_limit] = f0_45_min_max(torque_des, vel, pose, d_wind, v_wind, aoa_sail);
%         if upper_limit > deg2rad(45); upper_limit = deg2rad(45); end
%         if under_limit < deg2rad(-45); under_limit = deg2rad(-45); end
%         
%         str=['a_r:' num2str(alpha_rudder_r) ' N_Des:' num2str(torque_des) ' velx:' num2str(vel(1,1)) ' vely:' num2str(vel(2,1)) ' velz:' num2str(vel(3,1)) ' d_wind:' num2str(d_wind) ' v_wind:' num2str(v_wind) ' a_sail ' num2str(aoa_sail) ' under_li ' num2str(under_limit) ' upper_li ' num2str(upper_limit)];
%         disp(str)
%         
%         z = fzero(@(alpha_rudder_r) f0_45(alpha_rudder_r, torque_des, vel, pose, d_wind, v_wind, aoa_sail),start_search);
% %         z = deg2rad(8);
%         alpha_rudder_r_des = z;
%         if alpha_rudder_r_des >= deg2rad(45)
%             alpha_rudder_r_des = deg2rad(45);
%         elseif alpha_rudder_r_des <= deg2rad(-45)
%             alpha_rudder_r_des = deg2rad(-45);
%         end
%     end
% %             alpha_rudder_r_des = z;
% %         if alpha_rudder_r_des >= deg2rad(45)
% %             alpha_rudder_r_des = deg2rad(45);
% %         elseif alpha_rudder_r_des <= deg2rad(-45)
% %             alpha_rudder_r_des = deg2rad(-45);
% %         end
%     alpha_rudder_l_des = alpha_rudder_r_des;
%     
%     alpha_rudder_l      = rudderUpdate(alpha_rudder_l_des, alpha_rudder_l, delta_t, deg2rad(25));  %35 for delta_t = 0.2
%     alpha_rudder_r      = rudderUpdate(alpha_rudder_r_des, alpha_rudder_r, delta_t, deg2rad(25));
%     alpha_rudder_p      = [alpha_rudder_p alpha_rudder_r];
    %% control steps for position update
    
    if t == 0
        rcflags_sailorstate_requested     = 3;          % = AV_FLAGS_ST_NORMALSAILING
%         pose(1,1)                         =  1.111949403453934e+06;
%         pose(2,1)                         = -4.380225573914934e+06;
        vel(1,1)                          = 5;
        vel(2,1)			  = 0;
        vel(3,1)			  = 0.1;
        i=1;
% 	t_elaps_old=0;
    imu.position.latitude               = destData.Data(1).latitude;
    imu.position.longitude              = destData.Data(1).longitude;
        dist_boat                         =[sqrt((pose(1)-boat_x).^2+(pose(2)-boat_y).^2) zeros(1,5-num_boats)];
        dist_min                          = min(sqrt((pose(1)-boat_x).^2+(pose(2)-boat_y).^2));
    end
    
    for i=1:10
        dest_y(i) = r_earth*cos(destData.latitude*pi/180)*pi/180*(destData.Data(i).longitude-destData.longitude);
        dest_x(i) = r_earth*pi/180*(destData.Data(i).latitude-destData.latitude);
    end
    pose(2) = r_earth*cos(destData.latitude*pi/180)*pi/180*(imu.position.longitude-destData.longitude);
    pose(1) = r_earth*pi/180*(imu.position.latitude-destData.latitude)
    
    [pose, vel_p, vel, X, Y, N, X_p, Y_p, N_p, X_drag, Y_drag, X_waves, Y_waves, N_waves, X_sail, Y_sail, N_sail, N_rudder, N_damping, V_wind, g_r] = PoseStep(t, delta_t, pose, vel, X_p, Y_p, N_p, X_drag, Y_drag,vel_p, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, v_current, d_current, v_wind, d_wind, d_waves, T, h, depth, length, width, sail_factor);
    ax_lim=[-local_size/2+pose(2)-p2_0 local_size/2+pose(2)-p2_0 -local_size/2+pose(1)-p1_0 local_size/2+pose(1)-p1_0];
    pose
    11
    
    
    
    
    num_boats_temp = str2double(get(handles.num_boats,'String'));
    if num_boats_temp>num_boats
        boat_x(num_boats+1:num_boats_temp)        =ones(1,num_boats_temp-num_boats)*pose(1)+local_size/2; % [m]
        boat_y(num_boats+1:num_boats_temp)        =ones(1,num_boats_temp-num_boats)*pose(2)-local_size/2;  % [m]
        boat_heading(num_boats+1:num_boats_temp)  =deg2rad(120);%90*rand(1,num_boats_temp-num_boats)); % in rad
        boat_speed(num_boats+1:num_boats_temp)    =17;%(30 + (50-30)*rand(1,num_boats_temp-num_boats))*0.5144; % [m/s]   
    end
    if num_boats_temp<num_boats
        boat_x          =boat_x(1:num_boats_temp); % [m]
        boat_y          =boat_y(1:num_boats_temp);  % [m]
        boat_heading    =boat_heading(1:num_boats_temp); % in rad
        boat_speed      =boat_speed(1:num_boats_temp); % [m/s]   
    end
    num_boats=num_boats_temp;
    for i=1:num_boats
        if boat_x(i)-p1_0<ax_lim(3)
            boat_heading(i)=deg2rad(180*rand-90);
        end
        if boat_x(i)-p1_0>ax_lim(4)
            boat_heading(i)=deg2rad(-180*rand-90);
        end
        if boat_y(i)-p2_0<ax_lim(1)
            boat_heading(i)=deg2rad(180*rand);
        end
        if boat_y(i)-p2_0>ax_lim(2)
            boat_heading(i)=deg2rad(-180*rand);
        end
    end
    
    
    pose3_p                             = [pose3_p pose(3)];
    
    
    dist_dest=sqrt((pose(2))^2+(pose(1))^2);
    set(handles.dist_dest,'String',num2str(round(dist_dest)));
    
    wind_p    = [ones(1,5)*ax_lim(3)+local_size/20; ones(1,5)*ax_lim(1)+local_size/20] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    traj(n,:)                           = [(pose(1)-p1_0),(pose(2)-p2_0),pose(3)];
    

    boat_x      = boat_x + boat_speed.*cos(boat_heading)*delta_t;
    boat_y      = boat_y + boat_speed.*sin(boat_heading)*delta_t;

    boat_plot_x=zeros(6,0);
    boat_plot_y=zeros(6,0);
    for i=1:num_boats
        boat_plot_temp=point2boat([boat_y(i) boat_x(i) boat_heading(i)],50);
        boat_plot_x(:,i)=boat_plot_temp(:,1);
        boat_plot_y(:,i)=boat_plot_temp(:,2);
    end
    pose_plot=point2boat([pose(2) pose(1) pose(3)],50);
    if round(t)==100
        2324;
    end
    wp_x_loc=wp_x_glo+dest_x(destData.destNr+1);
    wp_y_loc=wp_y_glo+dest_y(destData.destNr+1);
    plot(handles.AxesTrajectory, pose_plot(:,1),pose_plot(:,2),'b',wp_y_loc,wp_x_loc,'ko',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', dest_y(destData.destNr+1), dest_x(destData.destNr+1),'sr',boat_plot_x,boat_plot_y,'r');%,wp_x,wp_y,'kx');%,wind(2,:),wind(1,:),'g');
    axis(handles.AxesTrajectory,[-world_size/2 world_size/2 -world_size/2 world_size/2])
    plot(handles.AxesLocal, pose_plot(:,1),pose_plot(:,2),'b',pose(2),pose(1),'b',boat_plot_x,boat_plot_y,'r',wp_y_loc,wp_x_loc,'ko-',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', dest_y(destData.destNr+1), dest_x(destData.destNr+1),'sr',wind_p(2,:),wind_p(1,:),'g')
    axis(handles.AxesLocal,ax_lim)
    %% plot desired heading on compass
    
    %     waypoint_1 = [220 10];
    %     dist_x = waypoint_1(1) - pose(1);
    %     dist_y = waypoint_1(2) - pose(2);
    %     dist_to_wp1 = sqrt( dist_x^2 + dist_y^2 );
    %    %     waypoint_1 = [220 10];
    %     dist_x = waypoint_1(1) - pose(1);
    %     dist_y = waypoint_1(2) - pose(2);
    %     dist_to_wp1 = sqrt( dist_x^2 + dist_y^2 );
    %     des_heading = atan2(dist_y,dist_x)-pi/2 - pose(3); %cos( (waypoint_1(1) - pose(1)) / dist_to_wp1 );   % - pose(3);
    % des_heading = atan2(dist_y,dist_x)-pi/2 - pose(3); %cos( (waypoint_1(1) - pose(1)) / dist_to_wp1 );   % - pose(3);
    des_heading     = reminderRad(des_heading);
    %
    arrow_length    = 1;
    des_heading_x   = arrow_length*sin( des_heading );           %  *cos(pose(3) + des_heading + pi/2)
    des_heading_y   = arrow_length*cos( des_heading );           %  *sin(pose(3) + des_heading + pi/2)
    pose_3_x        = arrow_length*sin( pose(3) );               %  + pi/2 fuer die ausrichtung
    pose_3_y        = arrow_length*cos( pose(3) );
    
    compass(handles.AxesDesiredHeading, des_heading_x,des_heading_y,'r')
    compass(handles.AxesPose3, pose_3_x, pose_3_y, 'b');
    
    
    %% set speed (along current orientation) in the gui
    
    avalon_speed_ms = sqrt(vel(1,1)^2 + vel(2,1)^2);    % [m/s]
    avalon_speed_kn = avalon_speed_ms/0.5144;       % [kn]
    set(handles.AvalonSpeed_ms,'String',num2str(round(avalon_speed_ms*10)/10));
    set(handles.AvalonSpeed_kn,'String',num2str(round(avalon_speed_kn*10)/10));
    
    
    %% write variables to DDX Store
    
    [imu.position.longitude, imu.position.latitude] = xy2ll(pose);
    if (~isempty(boat_x))
        dist_boat(n,:)=[sqrt((pose(1)-boat_x).^2+(pose(2)-boat_y).^2) zeros(1,5-num_boats)];
        
        dist_min(n)=min(dist_boat(n,find(dist_boat(n,:))));
        av_speed=sqrt(vel(1,1)^2+vel(2,1)^2);
        t_min=((pose(2)-boat_y).*(sin(boat_heading).*boat_speed-sin(pose(3)+atan2(vel(2),vel(1)))*av_speed)+(pose(1)-boat_x).*(cos(boat_heading).*boat_speed-cos(pose(3)+atan2(vel(2),vel(1)))*av_speed))./((boat_speed.*sin(boat_heading)-av_speed*sin(pose(3)+atan2(vel(2),vel(1))))^2+(boat_speed.*cos(boat_heading)-av_speed*cos(pose(3)+atan2(vel(2),vel(1))))^2);
        t_coll=t+t_min;
        curr_min_dist=sqrt((pose(2)+sin(pose(3)+atan2(vel(2),vel(1)))*av_speed.*t_min-boat_y-sin(boat_heading).*boat_speed*t_min).^2+(pose(1)+cos(pose(3)+atan2(vel(2),vel(1)))*av_speed.*t_min-boat_x-cos(boat_heading).*boat_speed.*t_min).^2);
        if t_min<0
            t_min=NaN;
            curr_min_dist=NaN;
        end
        if(round(t)==20)
        22;
%         pause
        end
    else
        dist_boat(n,:)=zeros(1,5);
        dist_min(n)= NaN;
        t_min=NaN;
        curr_min_dist=NaN;
    end
    
    set(handles.distance_out,'String',num2str(round(dist_min(end))));
    set(handles.exp_min_dist,'String',num2str(round(curr_min_dist)));
    set(handles.exp_time,'String',num2str(round(t_min)));
    
    ind_t=[0:delta_t:t+.01];
    plot(handles.AxesDist,ind_t, dist_boat)

    imu.attitude.yaw                                = rad2deg(pose(3));
    
    imu.velocity.x                      = vel(1,1)/0.5144;
    imu.velocity.y                      = -vel(2,1)/0.5144;
    imu.gyro.z                          = rad2deg(vel(3,1));
    
    desiredheading.heading              = rad2deg(des_heading);
    
    sailstate.degrees_sail              = rad2deg(reminderRad( aoa_sail ));
    rudderstateleft.degrees_rudder      = rad2deg(reminderRad(-alpha_rudder_l));
    rudderstateright.degrees_rudder     = rad2deg(reminderRad(-alpha_rudder_r));

    
    V_wind_x=(v_wind*cos(d_wind-pose(3))+vel(1));
V_wind_y=(v_wind*sin(d_wind-pose(3))+vel(2));
V_wind=sqrt((V_wind_x)^2+(V_wind_y)^2);
alpha=rad2deg(atan2(V_wind_y,V_wind_x)-aoa_sail);
    wind.speed                          = V_wind/0.5144;                 % v_wind                                        % [kn]
    wind.direction                      = rad2deg(atan2(V_wind_y,V_wind_x)-aoa_sail);%rad2deg(reminderRad(d_wind - pose(3) - aoa_sail));   % rad2deg(g_r - aoa_sail -pi)the windsensor is mounted on the mast, so the rot position has to be concidered
    wind.voltage                        = 12;                                           % randomly set to 12 Volt
    wind.temperature                    = 20;                                           % constant temp of 20°
    wind.uptodate                       = 2;                                            % wind Temp/Voltage up to date? -> 0:no,  else:yes
    wind.direction*pi/180;
    
    joystick.buttons(1)                 = joy_but1;
    joystick.buttons(2)                 = joy_but2;
    joystick.buttons(3)                 = joy_but3;
    joystick.buttons(4)                 = joy_but4;
    joystick.buttons(5)                 = joy_but5;
    joystick.buttons(6)                 = joy_but6;
    joystick.buttons(7)                 = joy_but7;
    joystick.buttons(8)                 = joy_but8;
    joystick.buttons(9)                 = joy_but9;
    joystick.axes(1)                    = joy_axes1;
    joystick.axes(2)                    = joy_axes2;
    joystick.axes(3)                    = joy_axes3;
    joystick.axes(4)                    = joy_axes4;
    
    for i=1:num_boats
        [boat_long boat_lat]                = xy2ll([boat_x(i);boat_y(i)]);

        aisData.Ship(i).mmsi                      = 111111*i;
        aisData.Ship(i).navigational_status       = 0;
        aisData.Ship(i).rate_of_turn              = 0;
        aisData.Ship(i).speed_over_ground         = round(boat_speed(i)/0.5144444);
        aisData.Ship(i).position_accuracy         = 1;
        aisData.Ship(i).longitude                 = boat_long;
        aisData.Ship(i).latitude                  = boat_lat;
        aisData.Ship(i).course_over_ground        = boat_heading(i)*180/pi;
        aisData.Ship(i).heading                   = boat_heading(i)*180/pi;
        aisData.Ship(i).destination(1)            = 0;
        aisData.Ship(i).time_of_arrival           = 0;
    end
    
    aisData.number_of_ships             = num_boats;
%     aisData.Ship(1)                     = aisStruct;
    
    %variables for the rcflags, if remotecontroll.cpp is not used !
    %----------------------------------------------------------------------------------------------------
    rcflags.emergency_stop              = rcflags_emergency_stop;
    rcflags.motion_stop                 = 0;
    rcflags.joystick_timeout            = 0;
    
    if flags_state == rcflags_sailorstate_requested;
        rcflags_sailorstate_requested   = 0;
    end
    
    rcflags.sailorstate_requested       = rcflags_sailorstate_requested;
    rcflags.man_in_charge               = 1; % = AV_FLAGS_MIC_SAILOR
    rcflags.autonom_navigation          = 1;
    
    ddx_write( avalon, wind, joystick, rudder, rcflags, sailstate, rudderstateright, rudderstateleft, desiredheading, imu, aisData)
    %% Plotting
%     
%     % traj(n,:) = [pose(1),pose(2)];
%     waveforces(n,:)     = [X_waves, Y_waves, N_waves];
%     sailforces(n,:)     = [X_sail, Y_sail, N_sail];
%     n=n+1;
%     
%     %     localsize_x     = pose(1);
%     %     localsize_y     = pose(2);
%     %     if abs(localsize_x) > local_size/2
%     %         local_size  = abs(pose(1))*2;
%     %     end
%     %     if abs(localsize_y) > local_size/2
%     %         local_size  = abs(pose(2))*2;
%     %     end
    
    
    psi     = pose(3);
    boat    = [ones(1,7)*(pose(1)-p1_0); ones(1,7)*(pose(2)-p2_0)] + ([-length/2,-length/2, length*1/4 ,length/2, length*1/4, -length/2, -length/2; 0, width/2, width/2, 0, -width/2 ,-width/2, 0]'*[cos(psi) sin(psi); -sin(psi) cos(psi)])';
    %    boat    = [ones(1,5)*(pose(1)-p1_0); ones(1,5)*(pose(2)-p2_0)] + ([0,-length/2, +length/2, -length/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(psi) sin(psi); -sin(psi) cos(psi)])';
    sailtree = [ones(1,2)*(pose(1)-p1_0); ones(1,2)*(pose(2)-p2_0)] + ([length*0.1, -length*0.4; 0 0]'*[cos(aoa_sail+pose(3)) sin(aoa_sail+pose(3)); -sin(aoa_sail+pose(3)) cos(aoa_sail+pose(3))])';
    rudder = [ones(1,2)*(pose(1)-p1_0); ones(1,2)*(pose(2)-p2_0)] + ([-length*1/2, -length*6.5/10; 0 0]'*[cos(alpha_rudder_r+pose(3)) sin(alpha_rudder_r+pose(3)); -sin(alpha_rudder_r+pose(3)) cos(alpha_rudder_r+pose(3))])';
    %     boat    = [(pose(1)-p1_0),(pose(2)-p2_0)];
    %     wind    = [ones(1,5)*(local_size/20-local_size/2); ones(1,5)*(local_size/20-local_size/2)] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    %wind = [ones(1,5)*(local_size/20-local_size/2); ones(1,5)*(local_size/20-local_size/2)] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_wind+pi) -sin(d_wind+pi); sin(d_wind+pi) cos(d_wind+pi)])';
    
    %waves   = [ones(1,5)*(local_size/20-local_size/2); ones(1,5)*(local_size/20-local_size/2)] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_waves+pi) sin(d_waves+pi); -sin(d_waves+pi) cos(d_waves+pi)])';
    %waves = [ones(1,5)*(local_size/20-local_size/2); ones(1,5)*(local_size/20-local_size/2)] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_waves+pi) -sin(d_waves+pi); sin(d_waves+pi) cos(d_waves+pi)])';
    
    drawnow;
    
    set(handles.currentTime,'String',num2str(t));
    t = t + delta_t;
    n=n+1;
    save data;
end

ddx_close( avalon );

%% additional plots

score   = size(X_p);

if t ~= score(2)*delta_t
    t   = score(2)*delta_t - delta_t;
end


% Forces
% hold(handles.AxesForces,'on');
% plot(handles.AxesForces, 0:delta_t:t, X_drag, 'r', 0:delta_t:t, Y_drag, 'g');
% legend(handles.AxesForces, 'F_{drag}', 'F_{lift}', 'Location', 'Best');
hold(handles.AxesForces,'on');
plot(handles.AxesForces, 0:delta_t:t, X_p, 'r', 0:delta_t:t, Y_p, 'g', 0:delta_t:t, N_p, 'b');%, traj(:,3),'o');
legend(handles.AxesForces, 'F_x', 'F_y', 'M_z', 'Location', 'Best');

% Velocity
hold(handles.AxesVelocity,'on');
plot(handles.AxesVelocity, 0:delta_t:t,vel_p(1,:),'r', 0:delta_t:t,vel_p(2,:),'g', 0:delta_t:t,vel_p(3,:),'b');
legend(handles.AxesVelocity, 'v_x', 'v_y', 'v_{rot}');

% Trajectory with waypoints
hold(handles.AxesTrajectoryWaypoint2,'on');
plot(handles.AxesTrajectoryWaypoint2, traj(:,2),traj(:,1));
axis(handles.AxesTrajectoryWaypoint2,'equal');

% Sailforces
% hold(handles.AxesSailforce,'on');
% plot(handles.AxesSailforce, 1:(n-1),sailforces(:,1),'r',1:(n-1),sailforces(:,2),'g');%,1:(n-1),sailforces(:,3),'b');
% legend(handles.AxesSailforce, 'Fsail_x', 'Fsail_y', 'Location', 'Best'); %', 'Ms_z'

% Rudder angle - Heading
hold(handles.AxesSailforce,'on');
plot(handles.AxesSailforce, 0:delta_t:t, rad2deg(pose3_p),'k', 0:delta_t:t, -rad2deg(alpha_rudder_p),'--');
legend(handles.AxesSailforce, 'Heading', 'Rudder_{angle}','Location', 'Best');

% Odometry
% hold(handles.AxesOdoAngle,'on');
% stairs(handles.AxesOdoAngle,data(:,9), data(:,4), 'b');
% stairs(handles.AxesOdoAngle,data(:,9), data(:,5), 'r');
% legend(handles.AxesOdoAngle,'Encoder (Right)', 'Encoder (Left)', 'Location', 'Best');
%hold(handles.AxesOdoAngle,'off');
% figure(4);
% plot(traj(:,2),traj(:,1));
% title('trajectory');
%
% figure(5);
% plot(1:(n-1),sailforces(:,1),'r',1:(n-1),sailforces(:,2),'g',1:(n-1),sailforces(:,3),'b')
%
% figure(6);
% plot(1:(n-1),waveforces(:,1),'r',1:(n-1),waveforces(:,2),'g',1:(n-1),waveforces(:,3),'b');
% title('sailforces');

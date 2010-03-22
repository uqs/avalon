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

%% State of Charge Battery

% hAx = handles.AxesSoC;%('Position',[.1 .1 .1 0.6]);
% SoC = 70;
% hPatch = thermometer(hAx,[0 20 100],SoC);
%% init variables
double d_wind
double v_wind
avalon = ddx_init();
parameter;

% theta_dot_star = [0 5 0 -5];

sail_factor = 0.1;
% set(handles.I_zEdit         ,'String' , num2str(I(3)));
% set(handles.A_hull3Edit     ,'String' , num2str(A_hull(3)));
% set(handles.Sail_factorEdit ,'String' , num2str(sail_factor))

%% Run Simulation with the Position Update Function PoseStep.m
alpha_rudder_r_des      = 0;
alpha_rudder_r          = 0;
alpha_rudder_l          = 0;
aoa_sail                = 0;
timer                   = 0;
des_heading             = 0;
p1_0                    = 4.578069e+06;  % lat,  y-axis
p2_0                    = -7.27922e+05;  % long, x-axis
% wypx                    = []
while (t < T_sim && ~Stop && ~rcflags_emergency_stop)
    %% read DDX Store variables
    
    %     [joystick, rudder, sail, flags, rcflags, desiredheading, imu, cleanimu, destStruct, destData, wypStruct, wypData ] = ddx_read( avalon );
    [joystick, rudder, sail, flags, rcflags, desiredheading, imu, cleanimu] = ddx_read( avalon );
    
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
    
    % des_heading         = deg2rad(desiredheading.heading);
    
    aoa_sail_des        = deg2rad(sail.degrees);           aoa_sail_des        = reminderRad(aoa_sail_des);        % sailTarget
    alpha_rudder_l_des  = deg2rad(-rudder.degrees_left);    alpha_rudder_l_des  = reminderRad(alpha_rudder_l_des);    % rudderTarget
    alpha_rudder_r_des  = deg2rad(-rudder.degrees_right);   alpha_rudder_r_des  = reminderRad(alpha_rudder_r_des);
    
    
    
    aoa_sail            = sailUpdate(aoa_sail_des, aoa_sail, delta_t, deg2rad(15));   aoa_sail        = reminderRad(aoa_sail);
    alpha_rudder_l      = rudderUpdate(alpha_rudder_l_des, alpha_rudder_l, delta_t, deg2rad(30));  %35 for delta_t = 0.2
    alpha_rudder_r      = rudderUpdate(alpha_rudder_r_des, alpha_rudder_r, delta_t, deg2rad(30));
    alpha_rudder_p      = [alpha_rudder_p alpha_rudder_r];
    flags_state                         = flags.state;
    rcflags_sailorstate_requested       = rcflags.sailorstate_requested;
    % destinationx                        = destData.latitude;
    % destinationy                        = destData.longitude;
    
    % wypx                                = wypData.Data.latitude;
    % wypy                                = wypData.Data.longitude;
    
    %% set sail state pop-up menu in GUI
    flags_state_string = {'IDLE     ';'DOCK     ';'NORMAL   ';'TACK     ';'JIBE     ';'UPWIND   ';'DOWNWIND ';'MAXENERGY';'HEADING C'};
    set(handles.SailStatePopupmenu,'String',flags_state_string(flags_state));
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
                set(handles.slider_rudderangle,'Value',0);
                set(handles.slider_rudderangle,'String','0');
            else
                set(handles.slider_rudderangle,'Value',d_rudderValue);
            end
            
            set(handles.rudder_angle_left,'String',num2str(rad2deg(alpha_rudder_l)));
            d_rudderValue_l = rad2deg(alpha_rudder_l);
            % set slider for rudder angle
            if (isempty(d_rudderValue_l) || d_rudderValue_l < -50 || d_rudderValue_l > 50)
                set(handles.slider_rudderangleLeft,'Value',0);
                set(handles.slider_rudderangleLeft,'String','0');
            else
                set(handles.slider_rudderangleLeft,'Value',d_rudderValue_l);
            end
        end
        alpha_rudder_l = alpha_rudder_r;
        
        % --------------------------------------------------------- sail angle by joystick
        aoa_sail = -joy_axes4/32767*pi;
        set(handles.sail_angle,'String',num2str(rad2deg(aoa_sail)));
        d_sailValue=rad2deg(aoa_sail);
        % set slider for sail angle
        if (isempty(d_sailValue) || d_sailValue <= -180 || d_sailValue >= 180)
            set(handles.slider_sailangle,'Value',0);
            set(handles.slider_sailangle,'String','0');
        else
            set(handles.slider_sailangle,'Value',d_sailValue);
        end
    else
        
        % ---------------------------------------------------------- set rudder
        set(handles.rudder_angle,'String',num2str(10*rad2deg(alpha_rudder_r)));
        d_rudderValue = 10*rad2deg(alpha_rudder_r);
        % set slider for rudder angle
        if (isempty(d_rudderValue) || d_rudderValue < -50 || d_rudderValue > 50)
            set(handles.slider_rudderangle,'Value',0);
            set(handles.slider_rudderangle,'String','0');
        else
            set(handles.slider_rudderangle,'Value',d_rudderValue);
        end
        
        set(handles.rudder_angle_left,'String',num2str(rad2deg(10*alpha_rudder_l)));
        d_rudderValue_l = 10*rad2deg(alpha_rudder_l);
        % set slider for rudder angle
        if (isempty(d_rudderValue_l) || d_rudderValue_l < -50 || d_rudderValue_l > 50)
            set(handles.slider_rudderangleLeft,'Value',0);
            set(handles.slider_rudderangleLeft,'String','0');
        else
            set(handles.slider_rudderangleLeft,'Value',d_rudderValue_l);
        end
        
        % ---------------------------------------------------------- set sail
        set(handles.sail_angle,'String',num2str(rad2deg(aoa_sail)));
        d_sailValue=rad2deg(aoa_sail);
        % set slider for sail angle
        if (isempty(d_sailValue) || d_sailValue < -185 || d_sailValue > 185)
            set(handles.slider_sailangle,'Value',0);
            set(handles.slider_sailangle,'String','0');
        else
            set(handles.slider_sailangle,'Value',d_sailValue);
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
        pose(1,1)                         = 4.578273e+06;
        pose(2,1)                         = -7.27919e+05;
        vel(1,1)                          = 0.5;
        
    end
    
%     I_z         = str2double(get( handles.I_zEdit           ,'String' ));
%     A_hull3     = str2double(get( handles.A_hull3Edit       ,'String' ));
%     sail_factor = str2double(get(handles.Sail_factorEdit    ,'String'));
    
%     [pose, vel_p, vel, X, Y, N, X_p, Y_p, N_p, X_waves, Y_waves, N_waves, X_sail, Y_sail, N_sail, N_rudder, N_damping, V_wind, g_r] = PoseStep(t, delta_t, pose, vel, X_p, Y_p, N_p, vel_p, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, v_current, d_current, v_wind, d_wind, d_waves, T, h, depth, length, width, I_z, A_hull3, sail_factor);
    [pose, vel_p, vel, X, Y, N, X_p, Y_p, N_p, X_drag, Y_drag, X_waves, Y_waves, N_waves, X_sail, Y_sail, N_sail, N_rudder, N_damping, V_wind, g_r] = PoseStep(t, delta_t, pose, vel, X_p, Y_p, N_p, X_drag, Y_drag,vel_p, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, v_current, d_current, v_wind, d_wind, d_waves, T, h, depth, length, width, sail_factor);
    
    pose3_p     = [pose3_p pose(3)];
%     set(handles.N_totEdit       ,'String',num2str(round(100*N)/100));
%     set(handles.N_rudderEdit    ,'String',num2str(round(100*N_rudder)/100));
%     set(handles.N_dampingEdit   ,'String',num2str(round(100*N_damping)/100));
%     set(handles.N_sailEdit      ,'String',num2str(round(100*N_sail)/100));
    
    p1_0                                = 4.578273e+06;  % long = -8.696893;
    p2_0                                = -7.27919e+05;  % lat  = 41.1734;
    %    destpoint_i                         = [(destinationx-p1_0) (destinationy-p2_0)];
    %    wyp                                 = [(wypx-p1_0) (wypy-p2_0)];
    
    wind    = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    traj(n,:)                           = [(pose(1)-p1_0),(pose(2)-p2_0),pose(3)];
    
    plot(handles.AxesTrajectory, traj(:,2),traj(:,1), wind(2,:),wind(1,:),'g');% , destpoint_i(2), destpoint_i(1),'o', wyp(2), wyp(1),'+');%,wind(2,:),wind(1,:),'g');
    
%     if timer <= 25;
%         plot(handles.AxesDotHeadingChange, rad2deg(alpha_rudder_r), rad2deg(vel(3,1)),'r','MarkerSize',8);
%         axis(handles.AxesDotHeadingChange, [-45 45 -15 15]);
%         hold on
%         timer = timer + delta_t;
%     else
%         timer = 0;
%         cla(handles.AxesDotHeadingChange);
%     end
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
    
    % set a constant step input for desired theta_dot
    % theta_i = mod(floor(t/70),4) + 1;
    % theta_dot_des = theta_dot_star(theta_i); % now = kappa_des !!!!!
    
    %% write variables to DDX Store
    
    [imu.position.longitude, imu.position.latitude] = xy2ll(pose);
    imu.attitude.yaw                                = rad2deg(pose(3));
    
    imu.velocity.x                      = vel(1,1)/0.5144;
    imu.velocity.y                      = -vel(2,1)/0.5144;
    imu.gyro.z                          = rad2deg(vel(3,1));
    % imu.theta_star                      = theta_dot_des;   % already in [deg/s]
    
    desiredheading.heading              = rad2deg(des_heading);
    
    sailstate.degrees_sail              = rad2deg(reminderRad( aoa_sail ));
    rudderstateleft.degrees_rudder      = rad2deg(reminderRad(-alpha_rudder_l));
    rudderstateright.degrees_rudder     = rad2deg(reminderRad(-alpha_rudder_r));

    wind.speed                          = V_wind/0.5144;                 % v_wind                                        % [kn]
    wind.direction                      = rad2deg(reminderRad(d_wind - pose(3) - aoa_sail));   % rad2deg(g_r - aoa_sail -pi)the windsensor is mounted on the mast, so the rot position has to be concidered
    wind.voltage                        = 12;                                           % randomly set to 12 Volt
    wind.temperature                    = 20;                                           % constant temp of 20°
    wind.uptodate                       = 2;                                            % wind Temp/Voltage up to date? -> 0:no,  else:yes
    
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
    
    ddx_write( avalon, wind, joystick, rudder, rcflags, sailstate, rudderstateright, rudderstateleft, desiredheading, imu) %desiredheading,
    %% Plotting
    
    % traj(n,:) = [pose(1),pose(2)];
    waveforces(n,:)     = [X_waves, Y_waves, N_waves];
    sailforces(n,:)     = [X_sail, Y_sail, N_sail];
    n=n+1;
    
    %     worldsize_x     = pose(1);
    %     worldsize_y     = pose(2);
    %     if abs(worldsize_x) > world_size/2
    %         world_size  = abs(pose(1))*2;
    %     end
    %     if abs(worldsize_y) > world_size/2
    %         world_size  = abs(pose(2))*2;
    %     end
    
    
    psi     = pose(3);
    boat    = [ones(1,7)*(pose(1)-p1_0); ones(1,7)*(pose(2)-p2_0)] + ([-length/2,-length/2, length*1/4 ,length/2, length*1/4, -length/2, -length/2; 0, width/2, width/2, 0, -width/2 ,-width/2, 0]'*[cos(psi) sin(psi); -sin(psi) cos(psi)])';
    %    boat    = [ones(1,5)*(pose(1)-p1_0); ones(1,5)*(pose(2)-p2_0)] + ([0,-length/2, +length/2, -length/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(psi) sin(psi); -sin(psi) cos(psi)])';
    sailtree = [ones(1,2)*(pose(1)-p1_0); ones(1,2)*(pose(2)-p2_0)] + ([length*0.1, -length*0.4; 0 0]'*[cos(aoa_sail+pose(3)) sin(aoa_sail+pose(3)); -sin(aoa_sail+pose(3)) cos(aoa_sail+pose(3))])';
    rudder = [ones(1,2)*(pose(1)-p1_0); ones(1,2)*(pose(2)-p2_0)] + ([-length*1/2, -length*6.5/10; 0 0]'*[cos(alpha_rudder_r+pose(3)) sin(alpha_rudder_r+pose(3)); -sin(alpha_rudder_r+pose(3)) cos(alpha_rudder_r+pose(3))])';
    %     boat    = [(pose(1)-p1_0),(pose(2)-p2_0)];
    %     wind    = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    %wind = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_wind+pi) -sin(d_wind+pi); sin(d_wind+pi) cos(d_wind+pi)])';
    
    %waves   = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_waves+pi) sin(d_waves+pi); -sin(d_waves+pi) cos(d_waves+pi)])';
    %waves = [ones(1,5)*(world_size/20-world_size/2); ones(1,5)*(world_size/20-world_size/2)] + ([0,-world_size/20, +world_size/20, -world_size/20, 0; 0, world_size/40, 0, -world_size/40, 0]'*[cos(d_waves+pi) -sin(d_waves+pi); sin(d_waves+pi) cos(d_waves+pi)])';
    
    %     if disp_forces == 1
    %         f = sqrt(X^2+Y^2);
    %         d_f = atan2(Y,X)+psi;
    %         forces = [ones(1,5)*(pose(1)-p1_0); ones(1,5)*(pose(2)-p2_0)] + ([0,+f, +f/2, f, f/2; 0, 0, -f/2, 0, f/2]'*[cos(d_f) sin(d_f); -sin(d_f) cos(d_f)])';
    %         moment = [];
    %         if N < 0
    %             for i=0:-pi/10:-pi
    %                 mom = [(pose(1)-p1_0)+N*cos(i); (pose(2)-p2_0)+N*sin(i)];
    %                 moment = [moment mom];
    %             end
    %             mom = [(pose(1)-p1_0)+N/2*cos(-pi*2/3) (pose(1)-p1_0)+N*cos(-pi) (pose(1)-p1_0)+3/2*N*cos(-pi*2/3); (pose(2)-p2_0)+N/2*sin(-pi*2/3) (pose(2)-p2_0)+N*sin(-pi) (pose(2)-p2_0)+3/2*N*sin(-pi*2/3)];
    %             moment = [moment mom];
    %         else
    %             for i=0:pi/10:pi  % O:pi/16:pi
    %                 mom = [(pose(1)-p1_0)+N*cos(i); (pose(2)-p2_0)+N*sin(i)];
    %                 moment = [moment mom];
    %             end
    %             mom = [(pose(1)-p1_0)+N/2*cos(pi*2/3) (pose(1)-p1_0)+N*cos(pi) (pose(1)-p1_0)+3/2*N*cos(pi*2/3); (pose(2)-p2_0)+N/2*sin(pi*2/3) (pose(2)-p2_0)+N*sin(pi) (pose(2)-p2_0)+3/2*N*sin(pi*2/3)];
    %             moment = [moment mom];
    %         end
    %     end
    
    %if disp_measurements == 1
    %   measurement = [ones(1,5)*gps_pose(1); ones(1,5)*gps_pose(2)] + ([0,-length/2, +length/2, -length/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(gps_pose(3)) sin(gps_pose(3)); -sin(gps_pose(3)) cos(gps_pose(3))])';
    % filtered = [ones(1,5)*x_hat(1); ones(1,5)*x_hat(2)] + ([0,-length/2, +length/2, -length/2, 0; 0, width/2, 0, -width/2, 0]'*[cos(x_hat(3)) sin(x_hat(3)); -sin(x_hat(3)) cos(x_hat(3))])';
    % mu = [x_hat(1),x_hat(2)];
    % ellipse=0:pi/30:2*pi; % angles around a circle
    % [eigvec,eigval] = eig([P(1,1:2);P(2,1:2)]); % Compute eigen-stuff
    % xyellipse = [cos(ellipse'),sin(ellipse')] * sqrt(eigval) * eigvec'; % Transformation
    % xellipse = xyellipse(:,1);
    % yellipse = xyellipse(:,2);
    %end
    
    
    if disp_forces == 1
        %if disp_measurements == 1
        %   plot(handles.AxesPose, boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g',measurement(2,:),measurement(1,:),'r',waypoint_1(2),waypoint_1(1),'+r');      %coordinate system is other way round
        %  axis(handles.AxesPose, [-world_size/2 world_size/2 -world_size/2 world_size/2]);
        %else
        %         plot(handles.AxesPose, wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g');%,forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g');      %coordinate system is other way round
        plot(handles.AxesPose, boat(2,:),boat(1,:),'k',sailtree(2,:),sailtree(1,:),'g',rudder(2,:),rudder(1,:),'--');%,',forces(2,:),forces(1,:),'r',moment(2,:),moment(1,:),'g');%boat(:,2),boat(:,1),'k',wind(2,:),wind(1,:),'r',moment(2,:),moment(1,:),'g');  % ,forces(2,:),forces(1,:),'r',waves(2,:),waves(1,:),'g'    %coordinate system is other way round
        axis(handles.AxesPose, 'equal'); %[-world_size/2 world_size/2 -world_size/2 world_size/2]);
        % end
        % %     else
        % %         if disp_measurements == 1
        % %             plot(handles.AxesPose, boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g',measurement(2,:),measurement(1,:),'r');      %coordinate system is other way round
        % %             axis(handles.AxesPose, [-world_size/2 world_size/2 -world_size/2 world_size/2]);
        % %         else
        % %             plot(handles.AxesPose, boat(2,:),boat(1,:),'b',wind(2,:),wind(1,:),'r',waves(2,:),waves(1,:),'g');      %coordinate system is other way round
        % %             axis(handles.AxesPose, [-world_size/2 world_size/2 -world_size/2 world_size/2]);
        % %         end
    end
    %     axis(handles.AxesPose, [-world_size/2 world_size/2 -world_size/2 world_size/2]);
    
    
    
    % plot(handles.AxesTrajectory, traj(:,2),traj(:,1));
    % axis(handles.AxesTrajectory,[-world_size/2 world_size/2 -world_size/2 world_size/2]);
    drawnow;
    
    set(handles.currentTime,'String',num2str(t));
    t = t + delta_t;
    
    %     t_gps = t_gps + delta_t;
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
hold(handles.AxesSailforce,'on');
plot(handles.AxesSailforce, 1:(n-1),sailforces(:,1),'r',1:(n-1),sailforces(:,2),'g');%,1:(n-1),sailforces(:,3),'b');
legend(handles.AxesSailforce, 'Fsail_x', 'Fsail_y', 'Location', 'Best'); %', 'Ms_z'

% Rudder angle - Heading
% hold(handles.AxesSailforce,'on');
% plot(handles.AxesSailforce, 0:delta_t:t, rad2deg(pose3_p),'k', 0:delta_t:t, -rad2deg(alpha_rudder_p),'--');
% legend(handles.AxesSailforce, 'Heading', 'Rudder_{angle}','Location', 'Best');

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

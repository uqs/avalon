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
d_wind=-pi/2;
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

% wypx                    = []
while (t < T_sim && ~Stop && ~rcflags_emergency_stop)
    %% read DDX Store vari5ables
%     boat_heading*180/pi
    [joystick, rudder, sail, flags, rcflags, desiredheading, imu, cleanimu, destStruct, destData, aisData, wypStruct, wypData] = ddx_read( avalon );
    if (round(t)==100)
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
    
    des_heading         = deg2rad(desiredheading.heading);
    delta_head=[delta_head rad2deg(des_heading-pose(3))];
    
    aoa_sail_des        = deg2rad(sail.degrees);           aoa_sail_des        = reminderRad(aoa_sail_des);        % sailTarget
    alpha_rudder_l_des  = deg2rad(-rudder.degrees_left);    alpha_rudder_l_des  = reminderRad(alpha_rudder_l_des);    % rudderTarget
    alpha_rudder_r_des  = deg2rad(-rudder.degrees_right);   alpha_rudder_r_des  = reminderRad(alpha_rudder_r_des);
    
    
    
    aoa_sail            = sailUpdate(aoa_sail_des, aoa_sail, delta_t, deg2rad(15));   aoa_sail        = reminderRad(aoa_sail);
    alpha_rudder_l      = rudderUpdate(alpha_rudder_l_des, alpha_rudder_l, delta_t, deg2rad(30));  %35 for delta_t = 0.2
    alpha_rudder_r      = rudderUpdate(alpha_rudder_r_des, alpha_rudder_r, delta_t, deg2rad(30));
    alpha_rudder_p      = [alpha_rudder_p alpha_rudder_r];
    (alpha_rudder_r-alpha_rudder_r_des)*1e+06;
    flags_state                         = flags.state;
    rcflags_sailorstate_requested       = rcflags.sailorstate_requested;
    i=1;
    while destData.Data(i).latitude~=0
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
wypData.Data(i).wyp_type;
k
wp_x_glo(i)=wypData.Data(i).latitude;
        wp_y_glo(i)=wypData.Data(i).longitude;
        wp_head(i)=wypData.Data(i).heading;
        i=2;
while (wypData.Data(i-1).wyp_type ~= 1)
        wp_x_glo(i)=wypData.Data(i).latitude;
        wp_y_glo(i)=wypData.Data(i).longitude;
        wp_head(i)=wypData.Data(i).heading;
        i=i+1;
        if i>100
            break
        end
end

    wp_x_loc=wp_x_glo-p1_0
    wp_y_loc=wp_y_glo-p2_0
    wp_head
%     wp_x=[wp_x_temp(1);nonzeros(wp_x_temp(2:end))]-p1_0;
%     wp_y=[wp_y_temp(1);nonzeros(wp_y_temp(2:end))]-p2_0;
%     wp_head=nonzeros(wp_head);
    destinationx                        = destData.latitude;
    destinationy                        = destData.longitude;
    
    
    %% set sail state pop-up menu in GUI
    flags_state_string = {'IDLE     ';'DOCK     ';'NORMAL   ';'TACK     ';'JIBE     ';'UPWIND   ';'DOWNWIND ';'MAXENERGY';'HEADING C'};
    set(handles.SailStatePopupmenu,'String',flags_state_string(flags_state));
    
       %% control steps for position update
    
    if t == 0
        rcflags_sailorstate_requested     = 3;          % = AV_FLAGS_ST_NORMALSAILING
        pose(1,1)                         =  1.111949403453934e+06;
        pose(2,1)                         = -4.380225573914934e+06;
        vel(1,1)                          = 1.5;
    end
    
    [pose, vel_p, vel, X, Y, N, X_p, Y_p, N_p, X_drag, Y_drag, X_waves, Y_waves, N_waves, X_sail, Y_sail, N_sail, N_rudder, N_damping, V_wind, g_r] = PoseStep(t, delta_t, pose, vel, X_p, Y_p, N_p, X_drag, Y_drag,vel_p, m, aoa_sail, A_sail, A_hull, A_rudder, alpha_rudder_r, alpha_rudder_l, C_d, C_hat, I, v_current, d_current, v_wind, d_wind, d_waves, T, h, depth, length, width, sail_factor);
    ax_lim=[-local_size/2+pose(2)-p2_0 local_size/2+pose(2)-p2_0 -local_size/2+pose(1)-p1_0 local_size/2+pose(1)-p1_0];    
    
    pose3_p                             = [pose3_p pose(3)];
    destpoint_i                         = [(destinationx-p1_0) (destinationy-p2_0)];
    
    dist_dest=sqrt((pose(2)-destinationy)^2+(pose(1)-destinationx)^2);
    set(handles.dist_dest,'String',num2str(round(dist_dest)));
    
    wind_p    = [ones(1,5)*ax_lim(3)+local_size/20; ones(1,5)*ax_lim(1)+local_size/20] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    traj(n,:)                           = [(pose(1)-p1_0),(pose(2)-p2_0),pose(3)];
    

   
    pose_plot=point2boat([pose(2) pose(1) pose(3)],50);
    if round(t)==100
        2324;
    end
    save data;
    plot(handles.AxesTrajectory, traj(:,2),traj(:,1), pose_plot(:,1)-p2_0,pose_plot(:,2)-p1_0,'b',wp_y_loc,wp_x_loc,'ko',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', destpoint_i(2), destpoint_i(1),'sr');%,wp_x,wp_y,'kx');%,wind(2,:),wind(1,:),'g');
    axis(handles.AxesTrajectory,[-world_size/2 world_size/2 -world_size/2 world_size/2])
    plot(handles.AxesLocal, pose_plot(:,1)-p2_0,pose_plot(:,2)-p1_0,'b',wp_y_loc,wp_x_loc,'ko',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', destpoint_i(2), destpoint_i(1),'sr',wind_p(2,:),wind_p(1,:),'g')
    axis(handles.AxesLocal,ax_lim)
    %% plot desired heading on compass
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

    imu.attitude.yaw                    = rad2deg(pose(3));
    imu.velocity.x                      = vel(1,1)/0.5144;
    imu.velocity.y                      = -vel(2,1)/0.5144;
    imu.gyro.z                          = rad2deg(vel(3,1));
    
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

%    joystick.axes(4)                    = joy_axes4;
    
    
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
    rudder=1;
    joystick=1;
    ddx_write( avalon, wind, joystick, rudder, rcflags, sailstate, rudderstateright, rudderstateleft, desiredheading, imu, aisData)
       drawnow;
    
    set(handles.currentTime,'String',num2str(t));
    t = t + delta_t;
    n=n+1;
    save data;
end

ddx_close( avalon );

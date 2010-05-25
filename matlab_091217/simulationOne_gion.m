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
timer                   = 0;
des_heading             = 0;
p1_0                    = 1.111949403453934e+06;  % lat,  y-axis
p2_0                    = -4.380225573914934e+06;  % long, x-axis
lat=p1_0;
long=p2_0;
speed=0.25;
simspeed = 12*0.51444;
head=90;
delta_head=[];
headingHistory=zeros(1,8);
simheading=0;
% wypx                    = []
while (t < T_sim && ~Stop && ~rcflags_emergency_stop)
    %% read DDX Store vari5ables
%     boat_heading*180/pi
    
[joystick, rudder, sail, flags, rcflags, desiredheading, imu, cleanimu, destStruct, destData, aisData, wypStruct, wypData, cleanwind] = ddx_read_g( avalon );
    if (round(t)==100)
%         pause;
2;
    end
    cleanwind.speed_long = 15.0/0.5144;
    cleanwind.global_direction_real = -90;
    cleanwind.bearing_real = reminderRad((cleanwind.global_direction_real - simheading)*pi/180)*180/pi;
            
    flags_state                         = flags.state;
    rcflags_sailorstate_requested       = rcflags.sailorstate_requested;
    
    for v=1:7
        headingHistory(v) = headingHistory(v+1);
    end
    headingHistory(8) = desiredheading.heading;
    heading_average = 0;
    for u=1:8
        heading_average = heading_average+1/8 * headingHistory(u);
    end
    simheading = reminderRad(deg2rad(4 + heading_average));
    
    long = long + cos(reminderRad(-simheading + pi/2))*simspeed;
    lat = lat + sin(reminderRad(-simheading + pi/2))*simspeed;
    
   
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
k;
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

    wp_x_loc=wp_x_glo-p1_0;
    wp_y_loc=wp_y_glo-p2_0;
    wp_head;
%     wp_x=[wp_x_temp(1);nonzeros(wp_x_temp(2:end))]-p1_0;
%     wp_y=[wp_y_temp(1);nonzeros(wp_y_temp(2:end))]-p2_0;
%     wp_head=nonzeros(wp_head);
    destinationx                        = destData.latitude;
    destinationy                        = destData.longitude;
    
    
    %% set sail state pop-up menu in GUI
    flags_state_string = {'IDLE     ';'DOCK     ';'NORMAL   ';'TACK     ';'JIBE     ';'UPWIND   ';'DOWNWIND ';'MAXENERGY';'HEADING C'};
    set(handles.SailStatePopupmenu,'String',flags_state_string(flags_state));
    
    [flags_state_string(flags_state) desiredheading.heading]
       %% control steps for position update
    
    if t == 0
        rcflags_sailorstate_requested     = 3;          % = AV_FLAGS_ST_NORMALSAILING
        
    end
    
    ax_lim=[-local_size/2+long-p2_0 local_size/2+long-p2_0 -local_size/2+lat-p1_0 local_size/2+lat-p1_0];    
    
    destpoint_i                         = [(destinationx-p1_0) (destinationy-p2_0)];
    
    dist_dest=sqrt((long-destinationy)^2+(lat-destinationx)^2);
    set(handles.dist_dest,'String',num2str(round(dist_dest)));
    
    wind_p    = [ones(1,5)*ax_lim(3)+local_size/20; ones(1,5)*ax_lim(1)+local_size/20] + ([0,-local_size/20, +local_size/20, -local_size/20, 0; 0, local_size/40, 0, -local_size/40, 0]'*[cos(d_wind+pi) sin(d_wind+pi); -sin(d_wind+pi) cos(d_wind+pi)])';
    traj(n,:)                           = [(lat-p1_0),(long-p2_0),simheading];
    

   
    pose_plot=point2boat([long lat simheading],50);
    if round(t)==100
        2324;
    end
    plot(handles.AxesTrajectory, traj(:,2),traj(:,1), pose_plot(:,1)-p2_0,pose_plot(:,2)-p1_0,'b',wp_y_loc,wp_x_loc,'ko',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', destpoint_i(2), destpoint_i(1),'sr');%,wp_x,wp_y,'kx');%,wind(2,:),wind(1,:),'g');
    axis(handles.AxesTrajectory,[-world_size/2 world_size/2 -world_size/2 world_size/2])
    plot(handles.AxesLocal, pose_plot(:,1)-p2_0,pose_plot(:,2)-p1_0,'b',wp_y_loc,wp_x_loc,'ko',wp_y_loc(k),wp_x_loc(k),'ks',dest_y,dest_x,'or', destpoint_i(2), destpoint_i(1),'sr',wind_p(2,:),wind_p(1,:),'g')
    axis(handles.AxesLocal,ax_lim)
    %% plot desired heading on compass
    des_heading     = reminderRad(deg2rad(desiredheading.heading));
    %
    arrow_length    = 1;
    des_heading_x   = arrow_length*sin( des_heading );           %  *cos(pose(3) + des_heading + pi/2)
    des_heading_y   = arrow_length*cos( des_heading );           %  *sin(pose(3) + des_heading + pi/2)
    pose_3_x        = arrow_length*sin( simheading );               %  + pi/2 fuer die ausrichtung
    pose_3_y        = arrow_length*cos( simheading );
    
    compass(handles.AxesDesiredHeading, des_heading_x,des_heading_y,'r')
    compass(handles.AxesPose3, pose_3_x, pose_3_y, 'b');
    
    
    %% set speed (along current orientation) in the gui
    
    avalon_speed_ms = sqrt(simspeed);    % [m/s]
    avalon_speed_kn = avalon_speed_ms/0.5144;       % [kn]
    set(handles.AvalonSpeed_ms,'String',num2str(round(avalon_speed_ms*10)/10));
    set(handles.AvalonSpeed_kn,'String',num2str(round(avalon_speed_kn*10)/10));
    
    
    %% write variables to DDX Store
    
    [imu.position.longitude, imu.position.latitude] = xy2ll([lat;long]);

    imu.attitude.yaw                    = rad2deg(simheading);
    
    

%    joystick.axes(4)                    = joy_axes4;
    
    
    %variables for the rcflags, if remotecontroll.cpp is not used !
    %----------------------------------------------------------------------------------------------------
%     rcflags.emergency_stop              = rcflags_emergency_stop;
%     rcflags.motion_stop                 = 0;
%     rcflags.joystick_timeout            = 0;
%     
%     if flags_state == rcflags.sailorstate_requested;
%         rcflags.sailorstate_requested   = 0;
%     end
    imu.speed=0.25;
%     rcflags.man_in_charge               = 1; % = AV_FLAGS_MIC_SAILOR
%     rcflags.motion_stop                 = 0;
%     rcflags.joystick_timeout            = 0;
%     
    if flags_state == rcflags_sailorstate_requested;
        rcflags_sailorstate_requested   = 0;
    end
    
    rcflags.sailorstate_requested       = rcflags_sailorstate_requested;
%     rcflags.man_in_charge               = 1; % = AV_FLAGS_MIC_SAILOR
    rcflags.autonom_navigation          = 1;
    rudder=1;
    joystick=1;
    ddx_write_g( avalon, rcflags,imu, cleanwind)
       drawnow;
    
    set(handles.currentTime,'String',num2str(t));
    t = t + delta_t;
    n=n+1;
    save data;
end

ddx_close( avalon );


addpath '/usr/local/lib/matlab/ddxmatlab'
addpath '.'

clear all;
%% initialize

avalon = ddx_init();

joystick_control = 1;
T_sim = 20000;
delta_t = 1/5;
t = 1;

alpha_rudder = deg2rad(0);
aoa_sail = deg2rad(20);

%% control loop
while t < T_sim
    
    [localization, control, wind, gps, rudderstate, sailstate, joystick] = ddx_read( avalon );
    
    joy_but1 = joystick.buttons(1);
    joy_but2 = joystick.buttons(2);
    joy_axes1 = double(joystick.axes(1));
    joy_axes2 = double(joystick.axes(2));
    joy_axes3 = double(joystick.axes(3));
    joy_axes4 = double(joystick.axes(4));
    
    
    if joystick_control == 1
        
        if joy_but1 == 1;      
            alpha_rudder = (joy_axes3/32767)*deg2rad(20);
        end
        
        aoa_sail = joy_axes4/32767*pi;
      
    end
    a_rudder = rad2deg(alpha_rudder)
    a_sail = rad2deg(aoa_sail)
    
    joystick.buttons(1) = joy_but1;
    joystick.buttons(2) = joy_but2;
    joystick.axes(1) = joy_axes1;
    joystick.axes(2) = joy_axes2;
    joystick.axes(3) = joy_axes3;
    joystick.axes(4) = joy_axes4;
    
    ddx_write( avalon, control, localization, wind, gps, rudderstate, sailstate, joystick );
    
    t = t+ delta_t;
    
end


%% close at the end
ddx_close( avalon )
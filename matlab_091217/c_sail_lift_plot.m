
clear all
close all
clc

for alpha_rudder = -pi/4:pi/500:pi/4;
    

    
   c_rudder_lift_appr = 1.9*(1-exp(-abs(alpha_rudder)*9))-2.4*abs(alpha_rudder);


    
    if abs(alpha_rudder) <= 2*pi/180            % Hysteresis around "dead into the wind"
        c_rudder_lift = 0;
        
    elseif abs(alpha_rudder) <= 25*pi/180       % 25deg = max c_sail_lift
        c_rudder_lift = (2.24*abs(alpha_rudder)-2.24*2*pi/180);   % sign(alpha_rudder)*
        
    elseif abs(alpha_rudder) > 25*pi/180 && abs(alpha_rudder) < 90*pi/180
        c_rudder_lift = -0.79*abs(alpha_rudder)+1.1;%0.9; % sign(alpha_rudder)*(-0.79*abs(alpha_rudder)+0.9)
        
    elseif abs(alpha_rudder) >= 90*pi/180 && abs(alpha_rudder) <= pi
        c_rudder_lift = 0;
    end
%     
    c_rudder_drag=1.28*sin(abs(alpha_rudder));
    subplot(2,1,1)
    plot(rad2deg(alpha_rudder) ,c_rudder_lift_appr,'b', rad2deg(alpha_rudder) ,c_rudder_drag,'+r')
    hold on
    subplot(2,1,2)
    plot(rad2deg(alpha_rudder) ,c_rudder_lift,'b', rad2deg(alpha_rudder) ,c_rudder_drag,'+r')
    hold on
    
end

hold off
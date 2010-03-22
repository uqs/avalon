% This function generates additional boats,
%
% and outputs the AIS signal
% -------------------------------------------------------------------------

clear all;
clc;

max_speed_kn = 20;                          % max speed of a boat is 25 kn
AisBoat_max = myrandint(1,1,[1:5]);              % max numbers of ships that can be around (btw 0 - 10)

T_sim = 200;                                 % [s]
t = 0;
delta_t = 1/4;                              % [s]

world_size = 100;

%ranInt_x = myrandint(i_max,1,[0:1]);
%ranInt_y = myrandint(i_max,1,[0:1]);
i=1;
n=1;

ais_length = 2;
ais_width = 1;

while i <= AisBoat_max
    
    % start point
    p.x(n,i) = -world_size/2;                           % start position of the boat: left upper cornern
    p.y(n,i) = world_size/2;
    
    % start angle and velocity
    s_phi(i) = -((pi/4-pi/10)*rand+pi/10);                 % rand start angle between [ 0 -  (-pi/4) ]
    s_speed(i) = max_speed_kn*0.5144*rand;                 % rand start speed [ 0 - 25*0.5144 m/s ]
    i = i + 1;
end
i=1;
%%

while t <= T_sim
    
    while i <= AisBoat_max
        
        p.x(n+1,i) = p.x(n,i) + s_speed(i)*delta_t*cos(s_phi(i));
        p.y(n+1,i) = p.y(n,i) + s_speed(i)*delta_t*sin(s_phi(i));
        
        if abs(p.x(n+1,i)) >= world_size/2
            s_phi(i) = -pi - s_phi(i);
        end
        
        if abs(p.y(n+1,i)) >= world_size/2
            s_phi(i) = -s_phi(i);
        end
        
        boat = [ones(1,5)*p.x((n+1),i); ones(1,5)*p.y((n+1),i)] + ([0,-ais_length/2, +ais_length/2, -ais_length/2, 0; 0, ais_width/2, 0, -ais_width/2, 0]'*[cos(s_phi(i)) sin(s_phi(i)); -sin(s_phi(i)) cos(s_phi(i))])';
        subplot(AisBoat_max,1,i)
        plot(boat(1,:),boat(2,:),'r');
        %plot(p.x(:,i),p.y(:,i));
        axis([-world_size/2 world_size/2 -world_size/2 world_size/2]);
        drawnow;
        
        ais_dist(n,i) = sqrt(( p.x(n+1,i)-1)^2 + (p.y(n+1,i)-1)^2);
        i=i+1;
    end
    

    
    i=1;
    t = t+delta_t;
    n=n+1;
    
end


% version 2 with change in speed and direction every t_change step
%
% while k <= k_max
%
%     % start point
%     p.x(n,k) = ranInt_x(1)*world_size/2;               % let the boat start on the left or on the right side
%     p.y(n,k) = ranInt_y(1)*world_size/2;               % let the boat start on a randomly choosen hight y
%
%     while t <= t_change*j
%
%         if t == t_change*j;
%             s_phi = abs(random('norm',0,pi/6));                    % rand start angle between [ 0 -  pi/6 ]
%             s_speed = abs(random('norm',0,max_speed_kn*0.5144));   % rand start speed [ 0 - 25*0.5144 m/s ]
%         end
%
%         p.x(n+1,k) = p.x(n,k) + s_speed*delta_t*cos(s_phi);
%         p.y(n+1,k) = p.y(n,k) + s_speed*delta_t*sin(s_phi);
%
%       %  new_pose = [start_pose; pose];
%         n=n+1;
%         t = t+delta_t;
%     end
%     j=j+1;
%     k = k + 1;
% end

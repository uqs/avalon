%% AIS Initialization

% max_speed_kn    = 20;                          % max speed of a boat is 25 kn
% AisBoat_max     = myrandint(1,1,[1:6]);        % max numbers of ships that can be around (btw 0 - 6)
%
% while j <= AisBoat_max
%
%     % start point
%     p.x(n,j)    = -world_size/2;                               % start position of the boat: left upper cornern
%     p.y(n,j)    = world_size/2;
%
%     % start angle and velocity
%     s_phi(j)    = -((pi/4-pi/15)*rand+pi/15);                  % rand start angle between [ pi/15 -  (-pi/4) ]
%     s_speed(j)  = max_speed_kn*0.5144*rand;                    % rand start speed [ 0 - 25*0.5144 m/s ]
%     j           = j + 1;
% end
%
% j=1;                                                           % set i back to '1' because its gona be used in an other loop later

%% simulated ais boats in the loop
    %     if ais_sim == 1
    %         k = 0;              % counter for the number of Ais boats
    %         while j <= AisBoat_max
    %
    %             p.x(n+1,j) = p.x(n,j) + s_speed(j)*delta_t*cos(s_phi(j));
    %             p.y(n+1,j) = p.y(n,j) + s_speed(j)*delta_t*sin(s_phi(j));
    %
    %             if abs(p.x(n+1,j)) >= world_size/2
    %                 s_phi(j) = -pi - s_phi(j);
    %             end
    %
    %             if abs(p.y(n+1,j)) >= world_size/2
    %                 s_phi(j) = -s_phi(j);
    %             end
    %
    %             % ais_boat = [ones(1,5)*p.x((n+1),j); ones(1,5)*p.y((n+1),j)] + ([0,-ais_length/2, +ais_length/2, -ais_length/2, 0; 0, ais_width/2, 0, -ais_width/2, 0]'*[cos(s_phi(j)) sin(s_phi(j)); -sin(s_phi(j)) cos(s_phi(j))])';
    %
    %             % subplot(i_max,1,i)
    %
    %             % plot(handles.AxesTrajectory, ais_boat(1,:),ais_boat(2,:),'r',traj(:,2),traj(:,1),'k');
    %             plot(handles.AxesTrajectory, p.x(n+1,j),p.y(n+1,j),'--rs',traj(:,2),traj(:,1),'k');
    %             axis(handles.AxesTrajectory,[-world_size/2 world_size/2 -world_size/2 world_size/2]);
    %             drawnow;
    %             hold on;
    %             % enumerate distance from current position to a ais_boat
    %
    %             ais_dist(n,j) = sqrt((p.x(n+1,j)-pose(1))^2 + (p.y(n+1,j)-pose(2))^2);
    %
    %             if ais_dist(n,j) <= ais_range
    %                 set(handles.AisSignalRadiobutton,'Value',1);
    %                 k = k + 1;                                      % numbers of detected ais_boats
    %                 speed_over_ground_Ais = s_speed(j);
    %                 heading_Ais = s_phi(j);
    %                 pose_Ais = [p.x(n+1,j); p.y(n+1,j)];
    %                 set(handles.AisNumberOfBoats,'String',num2str(k));
    %             else
    %                 set(handles.AisSignalRadiobutton,'Value',0);
    %                 set(handles.AisNumberOfBoats,'String',num2str(k));
    %
    %             end
    %
    %             j=j+1;
    %         end
    %         k = 0;
    %         j = 1;
    %     end
% generate a wind profile


clc
%% load the wind profile wind.mat
load wind

%% generate 2D map with arrows (wind magnitude, wind angle)

uneu=u(:,:,10);
vneu=v(:,:,10);
xneu=x(:,:,10);
yneu=y(:,:,10);
%% interpolation array
% x_v=xneu(1,1):0.1:xneu(1,end);
% y_v=yneu(1,1):0.1:yneu(end,1);
%% iteration and vector field generation
%  for i=1:41     % = 35
%  for j=1:35     % = 41
% %     vneuI(i,:) = interp1(xneu(i,:),vneu(i,:),x_v);
% %      uneuI(:,j) = interp1(yneu(:,j),uneu(:,j),y_v);
%      quiver(xneu(j,i), yneu(j,i), uneu(j,i), vneu(j,i), 0.05)    % generates arrows scalled by 0.05, uneuI, vneuI
%       hold on
%  end
%  end

% 
% clear all
% for i=1:40
% for j=1:40
% a{i,j}=[sin(2*pi/40*i) cos(2*pi/40*j)];
% end
% end
% for i=1:40
%     for j=1:40
% quiver(i ,j, a{i,j}(1), a{i,j}(2))
% hold on
%     end
% end

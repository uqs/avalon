function [winkel] = reminderRad(alpha)

alpha = mod(alpha+pi,2*pi)-pi;

% if alpha > pi
%     alpha = alpha - 2*pi;
% end
% 
% if alpha < -pi
%     alpha = alpha + 2*pi;
% end

winkel = alpha;




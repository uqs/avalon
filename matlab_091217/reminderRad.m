function [winkel] = reminderRad(alpha)

alpha = mod(alpha, 2*pi);
if alpha > pi
   	alpha = alpha - 2*pi;
end


winkel = alpha;




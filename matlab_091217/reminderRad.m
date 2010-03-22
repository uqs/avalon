function [winkel] = reminderRad(alpha)

if alpha > pi
    alpha = alpha - 2*pi;
end

if alpha < -pi
    alpha = alpha + 2*pi;
end

winkel = alpha;




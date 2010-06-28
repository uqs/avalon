function [alpha_rudder_neu] = rudderUpdate(alpha_rudder_des, alpha_rudder, delta_t, maxSpeed)

delta_r = abs(alpha_rudder_des-alpha_rudder);

if delta_r > (maxSpeed*delta_t)
    delta_r = maxSpeed*delta_t;
end

if alpha_rudder_des > alpha_rudder
    alpha_rudder_neu = alpha_rudder + delta_r;%*delta_t;
else
    alpha_rudder_neu = alpha_rudder - delta_r;%*delta_t;
end

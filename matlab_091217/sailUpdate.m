function [alpha_sail_neu] = sailUpdate(alpha_sail_des, alpha_sail, delta_t, maxSpeed)

delta_r = abs((alpha_sail_des - alpha_sail))*delta_t;

if abs(alpha_sail_des - alpha_sail) > pi
    if delta_r > (maxSpeed*delta_t)
        delta_r = maxSpeed*delta_t;
    end
    alpha_sail_neu = alpha_sail + sign(reminderRad(alpha_sail_des - alpha_sail))*delta_r;
else
    
    if delta_r > (maxSpeed*delta_t)
        delta_r = maxSpeed*delta_t;
    end
    
    if alpha_sail_des > alpha_sail
        alpha_sail_neu = alpha_sail + delta_r;
    else
        alpha_sail_neu = alpha_sail - delta_r;
    end
end

% This function generates a random gps signal 
% --------------------------------------------
% inputs are the current position pose
%
% output is the randomly generatet gps position gps_pose
%

function [gps_pose, t_gps, gps_length, gps_orient] = GpsSignal(t, pose, t_gps, sigma_gps, gps_length, gps_orient)

    update_gps = 1/4;       % [s]
    
    if t_gps >= update_gps
        gps_length_new = random('norm',0,sigma_gps);
        gps_orient_new = random('unif',0,2*pi);
        t_gps = 0;
    end
    
    % signal freez
    % ---------------------------------------------------------------------
    
    gps_signal_freeze = get(handles.GpsSignalFreezeRadiobutton,'Value');
    if gps_signal_freeze == 1;
        gps_length_new = 0;
        gps_orient_new = 0;
    end
    
    % random walk
    % ---------------------------------------------------------------------
    
    gps_random_walk = get(handles.GpsRandomWalkRadiobutton,'Value');
    if gps_random_walk == 1;
        gps_length_new = 20*random(1)*gps_length_new;
        gps_orient_new = 20*random(1)*gps_orient_new;
    end
    
    % drift
    % ---------------------------------------------------------------------
    
    gps_drift = get(handles.GpsDriftRadiobutton,'Value');
    if gps_drift == 1;
        gps_length_new = gps_length + 1;
        gps_orient_new = gps_orient + deg2rad(1);
    end
       
    % bias
    % ---------------------------------------------------------------------
    
    % gps_bias = get(handles.GpsBiasRadiobutton,'Value');
    

    % signal update, position update
    % ---------------------------------------------------------------------
    
    gps_length = gps_length_new;
    gps_orient = gps_orient_new;
        
    gps_pose = pose + [gps_length*cos(gps_orient); gps_length*sin(gps_orient); gps_orient];
    
    % power suply
    % ---------------------------------------------------------------------
    
    gps_power_supply = get(handles.GpsPowerSupplyRadiobutton,'Value');
    if gps_power_supply == 1;
        gps_pose = [];
    end
    
end
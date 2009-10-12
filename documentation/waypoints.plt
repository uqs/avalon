#display the logfiles (wypData and imu-Data)
plot 'imu' u (6371000.785 * cos($4*3.1415962/180)*(3.1415962/180)*$3):(6371000.785*(3.1415926/180)*$4) w l, 'wypData' u 2:3 w l

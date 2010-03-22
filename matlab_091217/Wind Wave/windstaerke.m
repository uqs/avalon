function [d_wind, l_wind] = windstaerke(x, y, uneu, vneu)



u1 = uneu(x,y);
v1 = vneu(x,y);

d_wind = atan2(v1,u1);
l_wind = sqrt(u1*u1+v1*v1);

end
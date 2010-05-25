function boat = point2boat(pose,size)

boat(1,1)=pose(1)-size*sqrt(5)*sin(pi/2+pose(3)-atan(2));
boat(1,2)=pose(2)-size*sqrt(5)*cos(pi/2+pose(3)-atan(2));
boat(2,1)=boat(1,1)+size*4*sin(pose(3));
boat(2,2)=boat(1,2)+size*4*cos(pose(3));
boat(3,1)=pose(1)+size*3*sin(pose(3));
boat(3,2)=pose(2)+size*3*cos(pose(3));
boat(5,1)=boat(1,1)+size*2*sin(pose(3)+pi/2);
boat(5,2)=boat(1,2)+size*2*cos(pose(3)+pi/2);
boat(4,1)=boat(5,1)+size*4*sin(pose(3));
boat(4,2)=boat(5,2)+size*4*cos(pose(3));
boat(6,1)=boat(1,1);
boat(6,2)=boat(1,2);


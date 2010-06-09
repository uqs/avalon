function ddx_write_shell( robot, wind, rudder, rcflags, sailstate, rudderstateright, rudderstateleft, imu, aisData)

%  DDXWrite( robot.control, control );
%  DDXWrite( robot.localization, localization );
 DDXWrite( robot.wind, wind );
%  DDXWrite( robot.cleanwind, cleanwind );
%  DDXWrite( robot.gps, gps);
%   DDXWrite( robot.joystick, joystick );
% DDXWrite( robot.rudder, rudder );
 % DDXWrite( robot.sail, sail );
 % DDXWrite( robot.flags, flags );
 DDXWrite( robot.rcflags, rcflags );
 % DDXWrite( robot.sailorflags, sailorflags );
 % DDXWrite( robot.naviflags, naviflags );
 % DDXWrite( robot.skipperflags, skipperflags );
 DDXWrite( robot.sailstate, sailstate );
 DDXWrite( robot.rudderstateright, rudderstateright );
 DDXWrite( robot.rudderstateleft, rudderstateleft );
%  DDXWrite( robot.desiredheading, desiredheading );
 DDXWrite( robot.imu, imu );
 % DDXWrite( robot.cleanimu, cleanimu );
%  DDXWrite( robot.aisStruct, aisStruct );
%  DDXWrite( robot.aisData, aisData );
 % DDXWrite( robot.destStruct, destStruct );
 % DDXWrite( robot.destData, destData );
%  DDXWrite( robot.wypStruct, wypStruct );
 % DDXWrite( robot.wypData, wypData );

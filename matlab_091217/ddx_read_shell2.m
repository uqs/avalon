% function [joystick, rudder, sail, flags, rcflags, desiredheading, imu, cleanimu, destStruct, destData, wypStruct, wypData ] = ddx_read( robot )
function [rudder, sail, flags, rcflags, destData, wypData, joystick, cleanimu, wind, cleanwind] = ddx_read_shell2( robot )%, 
  
  %localization     = DDXRead( robot.localization );
  %control          = DDXRead( robot.control );
  wind             = DDXRead( robot.wind );
  cleanwind        = DDXRead( robot.cleanwind );
  %gps              = DDXRead( robot.gps );
  joystick         = DDXRead( robot.joystick );
  rudder           = DDXRead( robot.rudder );
  sail             = DDXRead( robot.sail );
  flags            = DDXRead( robot.flags );  
  rcflags          = DDXRead( robot.rcflags );
  %sailorflags      = DDXRead( robot.sailorflags );
  %naviflags        = DDXRead( robot.naviflags );
  %skipperflags     = DDXRead( robot.skipperflags );
  %sailstate        = DDXRead( robot.sailstate );
  %rudderstateright = DDXRead( robot.rudderstateright );
  %rudderstateleft  = DDXRead( robot.rudderstateleft );
 % desiredheading   = DDXRead( robot.desiredheading );
 % imu              = DDXRead( robot.imu );
 cleanimu         = DDXRead( robot.cleanimu );
  %AisStruct        = DDXRead( robot.AisStruct );
 % aisData          = DDXRead( robot.aisData );
 % destStruct       = DDXRead( robot.destStruct );
 destData         = DDXRead( robot.destData );
 % wypStruct        = DDXRead( robot.wypStruct );
 wypData          = DDXRead( robot.wypData );


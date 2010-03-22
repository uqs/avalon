function ddx_close( robot )
  
  Done( robot.localization );
  Done( robot.control );
  Done( robot.wind );
  Done( robot.cleanwind );
  Done( robot.gps);
  Done( robot.joystick );
  Done( robot.rudder );
  Done( robot.sail );
  Done( robot.flags );
  Done( robot.rcflags );
  Done( robot.sailorflags );
  Done( robot.naviflags );
  Done( robot.skipperflags );
  Done( robot.sailstate );  
  Done( robot.rudderstateright );
  Done( robot.rudderstateleft );
  Done( robot.desiredheading );
  Done( robot.imu );
  Done( robot.cleanimu );
  Done( robot.AisStruct );
  Done( robot.AisData );
  Done( robot.destStruct );
  Done( robot.destData );
  Done( robot.wypStruct );
  Done( robot.wypData );
  
  clear robot.localization;
  clear robot.control;
  clear robot.wind;
  clear robot.cleanwind;
  clear robot.gps;
  clear robot.joystick;
  clear robot.rudder;
  clear robot.sail;
  clear robot.flags;
  clear robot.rcflags;
  clear robot.sailorflags;
  clear robot.naviflags;
  clear robot.skipperflags;
  clear robot.sailstate;
  clear robot.rudderstateright;
  clear robot.rudderstateleft;
  clear robot.desiredheading;
  clear robot.imu;
  clear robot.cleanimu;
  clear robot.AisStruct;
  clear robot.AisData;
  clear robot.destStruct;
  clear robot.destData;
  clear robot.wypStruct;
  clear robot.wypData;
  
  Disconnect( robot.store );
  clear robot.store;
  
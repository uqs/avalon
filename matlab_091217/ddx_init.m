function robot = ddx_init()

global robot_variable;

if isempty(robot_variable)
    
    LOCALIZATION_MSG     = 'struct { double x; double y; double z; double yaw; double pitch; double roll; double timestamp; } Localization';
    CONTROL_MSG          = 'struct { double xVelocity; double yVelocity; double rotVelocity; double timestamp; } Control';
    WINDDATA_MSG         = 'struct { double direction; double speed; double temperature; double voltage; int uptodate; } WindData';
    WINDCLEANDATA_MSG    = 'struct { double speed; double speed_long; double angle_of_attack_app ; double bearing_app; double bearing_real; double global_direction_app; double global_direction_real; double global_direction_real_long; } WindCleanData';
    GPSDATA_MSG          = 'struct { struct {double longitude; double latitude; double altitude;} position; double speed_kmph; double speed_kn; double course; short dgpsage; short dgpsref; float HDOP; short gpsFIX; int sat; } gpsData';
    JOYSTICK_MSG         = 'struct { int nb; int na; int axes[12]; int buttons[12];} joystick';
    RUDDERTARGET_MSG     = 'struct { double torque_des; double degrees_left; double degrees_right; int resetleft_request; int resetright_request;} rudderTarget';
    SAILTARGET_MSG       = 'struct { double degrees; int reset_request;} sailTarget';
    FLAGS_MSG            = 'struct { int man_in_charge; int state; int state_requested; int sail_direction; int sailor_no_tack_or_jibe; int navi_state; long navi_index_call; long navi_index_answer; int autonom_navigation; int global_locator; int joystick_timeout; int error_state; } Flags';
    RCFLAGS_MSG          = 'struct { int emergency_stop; int motion_stop; int joystick_timeout; int sailorstate_requested; int man_in_charge; int autonom_navigation; } rcFlags';
    SAILORFLAGS_MSG      = 'struct { int state; int no_tack_or_jibe; int sail_direction; } sailorFlags';
    NAVIFLAGS_MSG        = 'struct { int navi_state; long navi_index_call; long navi_index_answer; } NaviFlags';
    SKIPPERFLAGS_MSG     = 'struct { int global_locator; } SkipperFlags';
    SAILSTATE_MSG        = 'struct { float degrees_sail; float ref_sail; } Sailstate';
    RUDDERSTATERIGHT_MSG = 'struct { float degrees_rudder; float ref_rudder; } Rudderstate';
    RUDDERSTATELEFT_MSG  = 'struct { float degrees_rudder; float ref_rudder; } Rudderstate';
    DESIREDHEADING_MSG   = 'struct { double heading; } DesiredHeading';
    IMU_MSG              = 'struct { double speed; double theta_star; struct{ double longitude; double latitude; double altitude;} position; struct{ double roll; double pitch; double yaw;} attitude; struct{ double x; double y; double z;} velocity; struct{ double x; double y; double z;} acceleration; struct{ double x; double y; double z;} gyro; double temperature; } imuData';
    IMUCLEANDATA_MSG     = 'struct { struct { double roll; } attitude; struct { double x; double y; double z; double drift; } velocity; } imuCleanData';
    AISSTRUCT_MSG        = 'struct { unsigned long mmsi; unsigned int navigational_status; double rate_of_turn; unsigned int speed_over_ground; unsigned int position_accuracy; double longitude; double latitude; double course_over_ground; double heading; char destination[21]; double time_of_arrival; double timestamp;} AisStruct ';
    AISDATA_MSG          = 'struct { int number_of_ships; AisStruct Ship[15]; } AisData';
    OBSTACLE_MSG         = 'struct { double angle; double dist; double t_crit; double longitude; double latitude; } Obstacle';
    
    DESTINATIONSTRUCT_MSG   = 'struct { double longitude; double latitude; int passed; int type; } DestinationStruct';
    DESTINATIONDATA_MSG     = 'struct { DestinationStruct Data[1000]; double longitude; double latitude; int destNr; } DestinationData';
    WAYPOINTSTRUCT_MSG      = 'struct { double heading; int longitude; int latitude; int wyp_type; int passed; double windspeed; double winddirection; } WaypointStruct';
    WAYPOINTDATA_MSG        = 'struct { WaypointStruct Data[100]; } WaypointData';


        
       
    try
        % gpsData not yet complet
        out.store = DDXStore( 'localhost', 0, 1 );
        
        RegisterType( out.store, LOCALIZATION_MSG );
        RegisterType( out.store, CONTROL_MSG );
        RegisterType( out.store, WINDDATA_MSG );
        RegisterType( out.store, WINDCLEANDATA_MSG );
        RegisterType( out.store, GPSDATA_MSG );
        RegisterType( out.store, JOYSTICK_MSG );
        RegisterType( out.store, RUDDERTARGET_MSG );
        RegisterType( out.store, SAILTARGET_MSG );
        RegisterType( out.store, FLAGS_MSG );
        RegisterType( out.store, RCFLAGS_MSG );
        RegisterType( out.store, SAILORFLAGS_MSG );
        RegisterType( out.store, NAVIFLAGS_MSG );
        RegisterType( out.store, SKIPPERFLAGS_MSG );
        RegisterType( out.store, SAILSTATE_MSG );
        RegisterType( out.store, RUDDERSTATERIGHT_MSG );
        RegisterType( out.store, RUDDERSTATELEFT_MSG );
        RegisterType( out.store, DESIREDHEADING_MSG );
        RegisterType( out.store, IMU_MSG );
        RegisterType( out.store, IMUCLEANDATA_MSG );
        RegisterType( out.store, AISSTRUCT_MSG );
        RegisterType( out.store, AISDATA_MSG );
        RegisterType( out.store, OBSTACLE_MSG );
        RegisterType( out.store, DESTINATIONSTRUCT_MSG );
        RegisterType( out.store, DESTINATIONDATA_MSG );
       RegisterType( out.store, WAYPOINTSTRUCT_MSG );
       RegisterType( out.store, WAYPOINTDATA_MSG );
        
        RegisterVariable( out.store, 'localization', 'Localization' );
        RegisterVariable( out.store, 'control', 'Control' );
        RegisterVariable( out.store, 'wind', 'WindData' );
        RegisterVariable( out.store, 'cleanwind', 'WindCleanData');
        RegisterVariable( out.store, 'gps', 'gpsData' );
        RegisterVariable( out.store, 'joystick', 'joystick' );
        RegisterVariable( out.store, 'rudder', 'rudderTarget' );
        RegisterVariable( out.store, 'sail', 'sailTarget' );
        RegisterVariable( out.store, 'flags', 'Flags' );
        RegisterVariable( out.store, 'rcflags', 'rcFlags' );
        RegisterVariable( out.store, 'sailorflags', 'sailorFlags' );
        RegisterVariable( out.store, 'naviflags', 'NaviFlags' );
        RegisterVariable( out.store, 'skipperflags', 'SkipperFlags' );
        RegisterVariable( out.store, 'sailstate', 'Sailstate');
        RegisterVariable( out.store, 'rudderstateright', 'Rudderstate' );
        RegisterVariable( out.store, 'rudderstateleft', 'Rudderstate' );
        RegisterVariable( out.store, 'desiredheading', 'DesiredHeading' );
        RegisterVariable( out.store, 'imu', 'imuData' );
        RegisterVariable( out.store, 'cleanimu', 'imuCleanData' );
        RegisterVariable( out.store, 'aisStruct', 'AisStruct');
        RegisterVariable( out.store, 'aisData', 'AisData' );
        RegisterVariable( out.store, 'obstacle', 'Obstacle' );
        RegisterVariable( out.store, 'destStruct', 'DestinationStruct' );
        RegisterVariable( out.store, 'destData', 'DestinationData' );
       RegisterVariable( out.store, 'wypStruct', 'WaypointStruct' );
       RegisterVariable( out.store, 'wypData', 'WaypointData' );
        
        out.localization     = Variable( out.store, 'localization' );
        out.control          = Variable( out.store, 'control' );
        out.wind             = Variable( out.store, 'wind' );
        out.cleanwind        = Variable( out.store, 'cleanwind' );
        out.gps              = Variable( out.store, 'gps' );
        out.joystick         = Variable( out.store, 'joystick' );
        out.rudder           = Variable( out.store, 'rudder' );
        out.sail             = Variable( out.store, 'sail' );
        out.flags            = Variable( out.store, 'flags' );
        out.rcflags          = Variable( out.store, 'rcflags' );
        out.sailorflags      = Variable( out.store, 'sailorflags' );
        out.naviflags        = Variable( out.store, 'naviflags' );
        out.skipperflags     = Variable( out.store, 'skipperflags' );
        out.sailstate        = Variable( out.store, 'sailstate');
        out.rudderstateright = Variable( out.store, 'rudderstateright');
        out.rudderstateleft  = Variable( out.store, 'rudderstateleft');
        out.desiredheading   = Variable( out.store, 'desiredheading' );
        out.imu              = Variable( out.store, 'imu' );
        out.cleanimu         = Variable( out.store, 'cleanimu' );
        out.aisStruct        = Variable( out.store, 'aisStruct' );
        out.aisData          = Variable( out.store, 'aisData' );
        out.obstacle         = Variable( out.store, 'obstacle' );
        out.destStruct       = Variable( out.store, 'destStruct' );
        out.destData         = Variable( out.store, 'destData' );
       out.wypStruct        = Variable( out.store, 'wypStruct' );
       out.wypData          = Variable( out.store, 'wypData' );
        
        robot_variable = out;
        
    catch
        Disconnect( out.store );
        
    end
    
end

robot = robot_variable;



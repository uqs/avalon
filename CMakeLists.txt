cmake_minimum_required(VERSION 2.6)


# ADD_DEFINITIONS( -ggdb -Wall -O3 -fno-stack-protector )
ADD_DEFINITIONS( -ggdb -Wall )

INCLUDE_DIRECTORIES(
	 ${CMAKE_HOME_DIRECTORY}/eposlib /usr/local/include
)

LINK_DIRECTORIES(
	 ${CMAKE_HOME_DIRECTORY}/eposlib /usr/local/lib
)

# ADD_EXECUTABLE(windsensor windsensor.cpp)
# TARGET_LINK_LIBRARIES(windsensor ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(gps gps.cpp)
TARGET_LINK_LIBRARIES(gps ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(system-state system-state.cpp)
TARGET_LINK_LIBRARIES(system-state ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(remotecontrol remotecontrol.cpp)
TARGET_LINK_LIBRARIES(remotecontrol ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(ruddermain ruddermain.cpp RudderMotor.cpp)
TARGET_LINK_LIBRARIES(ruddermain ddxx ddx rtx epos can pthread rt )

ADD_EXECUTABLE(porttracker porttracker.cpp RudderMotor.cpp)
TARGET_LINK_LIBRARIES(porttracker ddxx ddx rtx  epos can pthread rt )

ADD_EXECUTABLE(eposdebug eposdebug.cpp RudderMotor.cpp SailMotor.cpp)
TARGET_LINK_LIBRARIES(eposdebug ddxx ddx rtx epos can pthread rt )

# ADD_EXECUTABLE(ais ais.cpp)
# TARGET_LINK_LIBRARIES(ais ddxx ddx rtx can epos pthread rt )

ADD_EXECUTABLE(aisEval aisEval.cpp)
TARGET_LINK_LIBRARIES(aisEval ddxx ddx rtx epos can pthread rt )

ADD_EXECUTABLE(sailor_transitions sailor_transitions.cpp Ship.cpp)
TARGET_LINK_LIBRARIES(sailor_transitions ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(sailor_statemachine sailor_statemachine.cpp Ship.cpp sailor_main_iter_fn.cpp sailor_rudder_iter_fn.cpp)
TARGET_LINK_LIBRARIES(sailor_statemachine gsl gslcblas ddxx ddx rtx pthread rt )

#ADD_EXECUTABLE(sailor_statemachine_testGB sailor_statemachine_testGB.cpp Ship.cpp)
#TARGET_LINK_LIBRARIES(sailor_statemachine_testGB ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(sailmain sailmain.cpp SailMotor.cpp)
TARGET_LINK_LIBRARIES(sailmain ddxx ddx rtx epos can pthread rt )

ADD_EXECUTABLE(sailmain_keepalive sailmain_keepalive.cpp)
TARGET_LINK_LIBRARIES(sailmain_keepalive ddxx ddx rtx pthread rt )

# Commented out while I don't have cmt
# ADD_EXECUTABLE(imu imu.cpp)
# TARGET_LINK_LIBRARIES(imu ddxx ddx rtx pthread rt cmt -fno-stack-protector)

ADD_EXECUTABLE(windcleaner windcleaner.cpp)
TARGET_LINK_LIBRARIES(windcleaner ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(imucleaner imucleaner.cpp)
TARGET_LINK_LIBRARIES(imucleaner ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(flag-checker flag-checker.cpp)
TARGET_LINK_LIBRARIES(flag-checker ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(testaufruf testaufruf.cpp)
TARGET_LINK_LIBRARIES(testaufruf ddxx ddx rtx pthread rt )

# ADD_EXECUTABLE(failure failure.cpp)
# TARGET_LINK_LIBRARIES(failure ddxx ddx rtx pthread rt )

# ADD_EXECUTABLE(skippertest skippertest.cpp)
# TARGET_LINK_LIBRARIES(skippertest ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(skipper skipper.cpp)
TARGET_LINK_LIBRARIES(skipper ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(global_skipper global_skipper.cpp)
TARGET_LINK_LIBRARIES(global_skipper ddxx ddx rtx pthread rt)
#TARGET_LINK_LIBRARIES(global_skipper ddxx ddx rtx pthread rt efence )

ADD_EXECUTABLE(skipper2 skipper2.cpp)
TARGET_LINK_LIBRARIES(skipper2 ddxx ddx rtx pthread rt )

## new files with Dijkstra:

ADD_EXECUTABLE(navigator navigator.cpp)
TARGET_LINK_LIBRARIES(navigator ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(simulator simulator.cpp)
TARGET_LINK_LIBRARIES(simulator ddxx ddx rtx pthread rt )

ADD_EXECUTABLE(obstacle_converter obstacle_converter.cpp)
TARGET_LINK_LIBRARIES(obstacle_converter)

ADD_EXECUTABLE(destination_converter destination_converter.cpp)
TARGET_LINK_LIBRARIES(destination_converter ddxx ddx rtx pthread rt) 

ADD_EXECUTABLE(dtom dtom.cpp)
TARGET_LINK_LIBRARIES(dtom)

ADD_EXECUTABLE(mtod mtod.cpp)
TARGET_LINK_LIBRARIES(mtod)

#ADD_EXECUTABLE(rootsolver_test rootsolver_test.cpp sailor_main_iter_fn.cpp sailor_rudder_iter_fn.cpp)
#TARGET_LINK_LIBRARIES(rootsolver_test gsl gslcblas rtx)


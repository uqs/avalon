#!/bin/bash
killall flag-checker
killall remotecontrol
killall porttracker
killall ruddermain
killall sailmain
killall imu
killall imucleaner
killall windsensor
killall windcleaner
killall sailor_transitions
killall sailor_statemachine_testGB
killall sailor_statemachine
killall skipper
killall skipper2
killall global_skipper
killall navigator
killall simulator

sleep 5
killall -9 flag-checker
killall -9 remotecontrol
killall -9 porttracker
killall -9 ruddermain
killall -9 sailmain
killall -9 imu
killall -9 imucleaner
killall -9 windsensor
killall -9 windcleaner
killall -9 sailor_transitions
killall -9 sailor_statemachine
killall -9 sailor_statemachine_testGB
killall -9 skipper
killall -9 skipper2
killall -9 global_skipper
killall -9 navigator
killall -9 simulator



#!/bin/bash
killall keepalive
killall flag-checker
killall remotecontrol
killall porttracker
killall ruddermain
killall sailmain
killall imu
killall imucleaner
killall windsensor
killall windcleaner
killall wind_faker
killall sailor_transitions
killall sailor_statemachine
killall destination_converter
killall global_skipper
killall skipper
killall navigator
killall poti
killall aisEval
killall launch
killall launchagent

sleep 5
killall -9 keepalive
killall -9 flag-checker
killall -9 remotecontrol
killall -9 porttracker
killall -9 ruddermain
killall -9 sailmain
killall -9 imu
killall -9 imucleaner
killall -9 windsensor
killall -9 wind_faker
killall -9 windcleaner
killall -9 sailor_transitions
killall -9 sailor_statemachine
killall -9 destination_converter
killall -9 global_skipper
killall -9 skipper
killall -9 navigator
killall -9 poti
killall -9 aisEval
killall -9 launch
killall -9 launchagent

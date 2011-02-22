#!/bin/sh

# list of programs to kill off
set -- keepalive flag-checker remotecontrol porttracker ruddermain sailmain \
    imu imucleaner windsensor windcleaner wind_faker sailor_transitions \
    sailor_statemachine destination_converter global_skipper skipper navigator \
    poti aisEval launch launchagent

pkill "$@"
sleep 5
pkill -9 "$@"

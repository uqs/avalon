#/bin/sh
while :; do
    ddxlog --maxtime 1m -sample 1.0 flags rcflags sailorflags naviflags sail rudder cleanwind sailstate imu desiredheading cleanimu wind && sleep 1
done

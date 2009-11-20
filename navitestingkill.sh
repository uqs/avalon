#/bin/bash
killall flag-checker &
killall remotecontrol &
killall simulator &
killall destination_converter &
killall global_skipper &
killall skipper &
killall navigator &

sleep 5
killall -9 flag-checker &
killall -9 remotecontrol &
killall -9 simulator &
killall -9 destination_converter &
killall -9 global_skipper &
killall -9 skipper &
killall -9 navigator &

#!/bin/sh
./flag-checker &
./remotecontrol &
./porttracker
./ruddermain -side left &
./ruddermain -side right &
./sailmain &
./imu &
./imucleaner &
#./windsensor &
#./keepalive ./windcleaner &
./wind_faker &
./sailor_transitions &
./sailor_statemachine &
#./keepalive ./sailor_statemachine &
#launch -c launch.conf &
#./destination_converter &
./global_skipper &
./skipper &
./navigator &
./poti &
#.:/aisEval &

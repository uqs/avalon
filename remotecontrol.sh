#!/bin/bash
./flag-checker &
./remotecontrol &
./porttracker
./ruddermain -side left &
./ruddermain -side right &
./sailmain &
./imu &
./imucleaner &
./windsensor &
./windcleaner &
./sailor_transitions &
#./sailor_statemachine_testGB &
./simulator &
./sailor_statemachine &
./destination_converter &
./global_skipper &
#./skipper &
./skipper2 &
./navigator &

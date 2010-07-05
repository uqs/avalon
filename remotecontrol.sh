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
./sailor_statemachine &
./destination_converter &
./global_skipper &
./skipper &
./navigator &
./poti &
./aisEval &

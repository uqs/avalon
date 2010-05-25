#!/bin/bash

launch -c launch.conf &
sleep 5
./destination_converter
sleep 1
matlab -nodisplay -r "addpath ~/Master_thesis/matlab/Matlab_obst_avoid_010410_new;simulationOne_shell; exit"


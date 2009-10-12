#************************************************************************#
#*									                                    *#
#*		             P R O J E K T    A V A L O N 		                *#
#*								                                    	*#
#*	    general_keepalive.py Watchdog for all the other processes       *#
#*								                                    	*#
#*	        Last Change	July 31, 2009; Hendrik Erckens  	            *#
#*									                                    *#
#************************************************************************#

import os
# import sys
import time
# import signal

# DDX Access
import ddxInterface

# Init DDX Interface
store = ddxInterface.ddxStore()

current_time = 0

class Variable:
    #TODO probably 'self' has to be removed in the next line
    def __init__(self,name,timeout,process):
        self.name = name
        self.timeout = timeout
        self.process = process
        self.var = store.variable(name)

    def check(self):
        # Check age of all the variables by comparing
        # their timestamps to the current time. Current time is updated
        # when a timestamp appears that is newer then the old current_time.
        global current_time
        self.var.read()
        ts = self.var.getTimeStamp()
        if (self.var.getTimeStamp() > current_time):
            current_time = self.var.getTimeStamp()

        age = current_time - self.var.getTimeStamp()
        timeout = float(self.timeout)
        # TODO check processes that don't write ddxVariables (such as store and
        # catalog differently with:
        # system_out = os.system("killall -0 " + self.process)
        return age < timeout

    def restart(self):
        os.system("killall -9 " + self.process)
        # print "bash -c '( " + self.process + " & )' &"
        # os.system("bash -c '( " + self.process + " & )' &")
        # start-stop-daemon -b --start --exec flag-checker --startas
        # /home/he/dokumente/ethz/fokusprojekt/segelboot/regelung/ssa/flag-checker
        print "start-stop-daemon -b --start --exec " + self.process + " --startas /home/he/dokumente/ethz/fokusprojekt/segelboot/regelung/ssa/" + self.process
        os.system("start-stop-daemon -b --start --exec " + self.process + " --startas /home/he/dokumente/ethz/fokusprojekt/segelboot/regelung/ssa/" + self.process)
        print self.process, "has just been restarted"
        # TODO add logging capability

        '''
        pid = os.fork()
        #execv("sh " + self.process + ".sh")
        if pid:
            # we are the parent
            signal.SIGCLD
            signal.SIG_IGN
        else:
            # we are the child
            os.system("sh " + self.process + ".sh &")
            sys.exit(0)
        '''



# format of config-file:
#
# rudder_state 1 ./ruddermain
# navigator 10 ./navigator
# ...

config_file = open("general_keepalive_config.txt","r")
log_file = open("general_keepalive_log.txt","r+")

lines = config_file.readlines() #.split("\n")
variables = {}

# read in the config file
for line in lines:
    # remove trailing newline ('\n') and split at spaces
    line = line.rstrip('\n')
    list = line.split(" ")
    print list
    # results in: ['flags', '5', './flag-checker.cpp']
    variables[list[0]] = Variable(list[0],list[1],list[2])

# initialize current time
current_time = 0

# Start the loop...
while True:
    for var in variables.values():
        time.sleep(0.1)
        if not var.check():
            var.restart()

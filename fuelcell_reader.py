##############################################################
#                                                            #
#               P R O J E K T    A V A L O N                 #
#                                                            #
#                                                            #
#        Script to read the values from the fuelcell         #
#                                                            #
#        (c) Stefan Wismer  -  wismerst@student.ethz.ch      #
#                                                            #
##############################################################

# Importing generic things
import ddxInterface
import serial

# And the Driver Class
import fuelcell

# Debug level: 0 is nothing, more is more (max: 3)
debug = 2

# FuelCell handler
sfc = fuelcell.FuelCell(debug)

# Global Stuff
msg = 'something that is not null'
msg_suc = False

if debug > 0:
    print "\n"
    print "PROJECT AVALON - Fuel Cell Values Reading Script"
    print "================================================\n\n"

    print "Fuel Cell script is running... Debug Mode is ON\n"

# Open Store
if debug > 0:
    print "Trying to Open DDX-Store..."

try:
    store = ddxInterface.ddxStore()

except:
    print "\n\n\nFailed to Connect to the DDX Store. Exiting..."
    exit()

else:
    if debug > 0:
        print "Sucessful.\n"

# Open a serial connection to the fuel cell...
sfc.serial_open("/dev/ttyUSB6",9600)

# Sending a linefeed to the fuel cell to wake it up
if debug > 0:
    print "Saying hello to the fuelcell..."

sfc.reset()

# Reading the values from the Fuel cell
sfc.get_values()

# TODO: Bring values to the store...

if debug > 0:
    print "Script ended."

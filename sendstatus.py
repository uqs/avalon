#############################################################################
#                                                                           #
#                       P R O J E C T    A V A L O N                        #
#                                                                           #
#        Script for sending Status Reports over SBD Service by Iridum       #
#                                                                           #
#                                                                           #
#        (c) Stefan Wismer  -  wismerst@student.ethz.ch                     #
#                                                                           #
#############################################################################

# Importing generic things
import ddxInterface
import serial
import sys

# And the Driver Class
import iridium

import message_manager

# Debug level: 0 is nothing, more is more (max: 3)
debug = 1

# Modem Handler
modem = iridium.IridiumModem(debug)

# Global Stuff
msg = 'something that is not null'
msg_suc = False
modem_ready = False             # has the pincode been entered correctly?
pin_rqd = 3                     # which pin? (0 = none, 1 = 1, 2 = 2, more = unkonwn) 

if debug > 0:
    print "\n"
    print "PROJECT AVALON - Status Reporting Script"
    print "========================================\n\n"

    print "Status reporting Script is running... Debug Mode is on\n"

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

# Read all the values

if debug > 0:
    print "Gathering all required data from the store..."

# Position data
if debug > 1:
    print "Reading IMU Data..."
IMUData = store.variable("imu")
IMUData.read(5.0,1);


# Windsensor data (for Voltage...)
if debug > 1:
    print "Reading Windsensor Data..."
have_wind = 0
try:
    WindData = store.variable("wind")
    WindData.read(5.0,1);
    have_wind = 1
except:
    have_wind = 0

dump = 0.0

# compile into string, terminate with nice greeting... (on multiple lines for
# readablility...
if have_wind == 0:
    text = "%.6f,%.6f,%.1f,%.1f" % (float(IMUData.position.longitude), float(IMUData.position.latitude), dump, dump) # float(WindData.voltage), float(WindData.temperature))
else:
    text = "%.6f,%.6f,%.1f,%.1f" % (float(IMUData.position.longitude), float(IMUData.position.latitude), float(WindData.voltage), float(WindData.temperature))

text = "%s,0.00,0.0,%.1f,0,0,0,0,0,0,00000000,cheers avalon" % (text,IMUData.attitude.yaw)

if debug > 0:
    print ("Done.\n\nMessage to be sent: %s") % text

message_manager.send_status(text)
sys.exit()

# Open a serial connection to the modem...
modem.serial_open("/dev/ttyUSB7",2400)

# Reset it
modem.reset()

# Enter PIN - Code
# modem.enter_pin("1111", "1234")

# Get a pdu string
pdu = modem.compile_sms(text)

# send sms to the pre-programmed number
modem.send_sms(pdu)

# Ending Note... (debug level -1)
print "Script completed. Bye! \n\n"

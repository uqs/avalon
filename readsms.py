##############################################################
#                                                            #
#               P R O J E K T    A V A L O N                 #
#                                                            #
#                                                            #
#        Script for reading sms and interpreting commands    #
#                                                            #
#        (c) Stefan Wismer  -  wismerst@student.ethz.ch      #
#                                                            #
##############################################################


# Importing generic things
import ddxInterface

# And the Driver Class
import iridium

# Debug level: 0 is nothing, more is more (max: 3)
debug = 1

# Modem Handler
modem = iridium.IridiumModem(debug)

# Global Stuff
msg = 'something that is not null'

if debug > 0:
    print "\n"
    print "PROJECT AVALON - SMS Receiving Script"
    print "=====================================\n\n"

    print "SMS Receiving Script is running... Debug Mode is on\n"

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

# Open a serial connection to the modem...
modem.serial_open("/dev/ttyUSB0",2400)

# Reset it
modem.reset()

# Enter PIN - Code
# modem.enter_pin("1111", "1234")

new_sms = modem.get_num_new_sms()
if debug > 0:
    print "There are %d new sms!" % new_sms

if new_sms > 0:
    max_count = modem.get_newest_sms_count()
    if debug > 0:
        print "Newest SMS has count %d" % max_count

for i in range(0,new_sms):
    text = modem.read_sms(max_count - new_sms + i + 1)

    if debug > 0:
        print text

# Ending Note... (debug level -1)
print "Script completed. Bye! \n\n"

############################################################
#                                                          #
#        Test script for SMS fuctionality of modem         #
#                                                          #
#        Author: Stefan Wismer                             #
#                wismerst@student.ethz.ch                  #
#                                                          #
############################################################

small script to test the functionality of the iridium-module

import iridium

modem = iridium.IridiumModem(3)

# bla = modem.compile_sms("Hello World!")
# print "Complete PDU-String is %s" % bla

modem.serial_open("/dev/ttyUSB0",2400)

modem.reset()
print "there are %d sms of which %d are new." % (modem.get_num_sms(),
        modem.get_num_new_sms())

print "hightest count is %d" % modem.get_newest_sms_count()

print "testsms got \"%s\"" % modem.read_sms(4)
# print chr(n)
# print "Testsms: Modem message count is %d" % bla

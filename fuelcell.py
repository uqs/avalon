############################################################
#                                                          #
#        Driver Class for the SFC 1200 Fuel cell           #
#                                                          #
#        Author: Stefan Wismer                             #
#                wismerst@student.ethz.ch                  #
#                                                          #
############################################################

import serial
import time
import string

# Handle to serial interface
ser = serial.Serial()

class FuelCell:

    def __init__(self,debug):
        self.debug = debug
        if self.debug > 1:
            print "Fuel Cell Class initated..."

    def __del__(self):
        if self.debug > 0:
            print "Closing Serial Connection..."
        ser.close

    def serial_open(self, commport, speed):
        if self.debug > 1:
            print "Class: serial_open called with %s and %d" % (commport,speed)

        # Open Serial Connection
        if self.debug > 0:
            print "Trying to open serial port %s with speed %d..." % (commport,speed)

        ser.port = commport
        ser.baudrate = speed
        ser.timeout = 1

        try: 
            ser.open()

        finally:
            if ser.isOpen() == True:
                if self.debug > 0:
                    print "Successful.\n"

            else:
                if self.debug > 0:
                    print "Failed. Exiting...\n"
                    exit()

    def reset(self):

        msg = 'something that is not null'
        msg_suc = False

        if self.debug > 0:
            print "Sending Linefeed ..."

        ser.write('\n\r')

        while msg != '':
            msg = ser.readline()
            if self.debug > 2:
                print msg

            if msg.count('SFC'):
                msg_suc = True

        if self.debug > 0:
            if msg_suc == True:
                print "Successful.\n"
            else :
                print "Failed. Exiting...\n"
                exit()

    def get_values(self):

        msg = 'something that is not null'
        msg_suc = False

        if self.debug > 0:
            print "Sending SFC ..."

        ser.write('SFC\r')

        msg = ser.readline()
        lines = msg.split('\r')

        for i in range(0,len(lines)): 
            if self.debug > 2:
                print lines[i]

            if lines[i].count("voltage"):
                msg_suc = True
                word = lines[i].strip("battery voltage ")
                voltage = word.strip("V")
                print "The voltage is %s" % voltage

        if self.debug > 0:
            if msg_suc == True:
                print "Successful.\n"
            else :
                print "Failed. Exiting...\n"
                exit()


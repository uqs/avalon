############################################################
#                                                          #
#        Driver Class for the IRIDIUM 9522-B Modem         #
#                                                          #
#        Author: Stefan Wismer                             #
#                wismerst@student.ethz.ch                  #
#                                                          #
############################################################

# All the things we need
import serial
import time
import string
import math

# Handle to serial interface
ser = serial.Serial()

class IridiumModem:

    def __init__(self, debug):
        self.debug = debug
        if self.debug > 1:
            print "Modem Class initiated..."

    def __del__(self):
        if self.debug > 0:
            print "Closing Serial Connection..."
        ser.close()

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
            print "Sending ATZ ..."

        ser.write('atz\n\r')
        time.sleep(1)

        while msg != '' :
            msg = ser.readline()
            if self.debug > 2:
                print msg

            if msg.count('OK'):
                msg_suc = True

        if self.debug > 0:
            if msg_suc == True:
                print "Successful.\n"
            else :
                print "Failed. Exiting...\n"
                exit()

    def enter_pin(self, pin1, pin2):

        msg = 'something that is not null'
        msg_suc = False
        modem_ready = False             # has the pincode been entered correctly?
        pin_rqd = 3                     # which pin? (0 = none, 1 = 1, 2 = 2, more = unkonwn) 

        if self.debug > 0:
            print "Trying to Enter PIN-Code..."


        while modem_ready == False:
            
            ser.write('at+cpin?\r')
            msg = 'something'

            while msg != '':
                msg = ser.readline()
                if self.debug > 2:
                    print msg

                if msg.count('READY') == 1 :
                    if self.debug > 0 :
                        print "Successful.\n"

                    pin_rqd = 0;
                    modem_ready = True;

                elif msg.count('SIM PIN\r') == 1:
                    if self.debug > 1:
                        print "PIN-Code 1 Requested."
                    
                    pin_rqd = 1;
                
                elif msg.count('SIM PIN2') == 1:
                    if self.debug > 1:
                        print "PIN-Code 2 Requested."

                    pin_rqd = 2;

            # Enter the prespective pin...

            if pin_rqd >2:
                if self.debug > 0:
                    print "Failed. Exiting..."
                    exit()

            elif pin_rqd == 1:
                ser.write('at+cpin=\"1111\"\r')
                if self.debug > 1:
                    print "Entering PIN 1 ..."

                # wait for pin to be accepted
                time.sleep(3)

                msg = 'something'
                msg_suc = False

                while msg != '':
                    msg = ser.readline()
                    if self.debug > 2:
                        print msg

                    if msg.count('OK') == 1:
                        msg_suc = True
                        if self.debug > 1:
                            print "PIN 1 accepted."

                if msg_suc == False :
                    if self.debug > 1:
                        print "Failed to enter pin 1. Exiting..."
                        exit()

            elif pin_rqd == 2:
                ser.write('at+cpin=\"2222\"\r')
                if self.debug > 1:
                    print "Entering PIN 2 ..."

                # wait for pin to be accepted
                time.sleep(3)

                msg = 'something'
                msg_suc = False

                while msg != '':
                    msg = ser.readline()
                    if self.debug > 2:
                        print msg

                    if msg.count('OK') == 1:
                        msg_suc = True
                        if self.debug > 1:
                            print "PIN 2 accepted."

                if msg_suc == False :
                    if self.debug > 1:
                        print "Failed to enter pin 2. Exiting..."
                        exit()

    def write_message_to_buffer(self, text):
        if self.debug > 0:
            print "Sending Message String to MO-Buffer..."

        ser.write('at+sbdwt=' + text + '\r')

        msg = 'something'
        while msg != '' :
            msg = ser.readline()
            if self.debug > 2:
                print msg

            if msg.count('OK'):
                msg_suc = True

        if self.debug > 0:
            if msg_suc == True:
                print "Successful.\n"
            else :
                print "Failed. Exiting...\n"
                exit()

    def send_message(self):
        # TODO: This has to be implemented at some point...
        print "Sending Buffer..."

    def compile_sms(self, text):
        if self.debug > 0:
            print "Encoding PDU message..."

        # This countains the service center number and destination number
        # (currently 0794467176 for testing...)
        header = "079188612609005011000B911497447671F60000AA"

        # get length of text and format nicely...
        len_oct = "%02X" % len(text)
        if self.debug > 2:
            print "Adding length of text = %s to the header..." % (len_oct)

        # Make blocks of seven bits, and invert them
        s = []
        for i in range(0,len(text)):
            b = ord(text[i])
            for j in range(0,7):
                s.append(b & 1)
                b = b >> 1

        if self.debug > 2:
            print "String in inverted 7blocks:"
            print s        

        # add zeroes to the next number which is a mulititude of 8
        for i in range(0,8*int(math.ceil((len(s)/8.0)))-len(s)):
            s.append(0)

        if self.debug > 2:
            print "Zeros have been appended to fit, new length: %d" % len(s)

        # now pick blocks of 8 and reverse, save result in data
        z = []
        data = ""
        for i in range(0,len(s)/8):
            p = s[i*8:i*8+8]
            z.append(0)
            for j in range(0,8):
                z[i] = z[i] + p[7-j]*pow(2,7-j)

            o = "%02X" % z[i]
            data = "%s%s" % (data, o)

        if self.debug > 2:
            print "Text in PDU-Code is %s" % data

        # return the combination of header, length and user data
        return "%s%s%s" % (header, len_oct, data)

    def send_sms(self, pdustring):
        msg = 'something'

        if self.debug > 0:
            print "Sending SMS..."

        ser.write("at+cmgs=%d\r" % (len(pdustring)/2))

        if self.debug > 1:
            print "AT+CMGS sent, waiting for > ... ",

        while msg != '':
            msg = ser.readline()
            if self.debug > 2:
                print "\n%s" % msg
            if msg.count('>') == 1:
                print "Got it!"

        if self.debug > 1:
            print "Got it."

        ser.write(pdustring)
        ser.write(chr(26))

        if self.debug > 1:
            print "Ctrl-Z sent. waiting for answer..."

        msg = 'something'
        while msg != '':
            msg = ser.readline()
            if self.debug > 2:
                print msg
            if msg.count("OK") == 1:
                if self.debug > 0:
                    print "SEND SUCCESSFUL"
                return 0
            if msg.count("ERROR") == 1:
                if self.debug > 0:
                    print "SEND FAILED"
                return 1

    def list_sms(self, param):
        # this asks for a sms-list and returns  the number or messages.

        msg = 'something'

        if self.debug > 0:
            print "Reading sms list..."

        if self.debug > 1:
            print "Sending AT+CMGL..."
        ser.write("at+cmgl=%d\r" % param)

        i = 0
        while msg != '':
            msg = ser.readline()
            if self.debug > 2:
                print msg
            if msg.count("OK") == 1:
                if self.debug > 0:
                    print "Listed sms successfully. Message count is %d." % i
                return i
            if msg.count("+CMGL:") == 1:
                if self.debug > 1:
                    print "Message deteced..."
                i = i+1
            if msg.count("ERROR") == 1:
                if self.debug > 0:
                    print "Something failed. Exiting..."
                return -1

    def get_num_sms(self):
        return self.list_sms(4)

    def get_num_new_sms(self):
        return self.list_sms(0)

    def get_newest_sms_count(self):
        msg = 'something'

        if self.debug > 0:
            print "Reading sms list to get the count..."

        if self.debug > 1:
            print "Sending AT+CMGL..."
        ser.write("at+cmgl=4\r")

        i = 0
        while msg != '':
            msg = ser.readline()
            if self.debug > 2:
                print msg
            if msg.count("OK") == 1:
                if i == 0:
                    if self.debug > 0:
                        print "No sms in list. Returning 0..."
                    return i
                else:
                    if self.debug > 0:
                        print "Success."
                    return i
            if msg.count("+CMGL:") == 1:
                if self.debug > 1:
                    print "Message deteced..."
                msg.rstrip("+CMGL:")
                i = int(msg[6:9])
            if msg.count("ERROR") == 1:
                if self.debug > 0:
                    print "Something failed."
                return 0

    def read_sms(self,num):
        # this returns content of message nummerber "num" in clear text

        msg = 'something'
        finished = False
        sms_text = ""
        s = []

        if self.debug > 0:
            print "Going to parse sms number %d" % num

        ser.write("at+cmgr=%d\r" % num)

        if self.debug > 1:
            print "AT+CMGR sent. Waiting for answer..."

        while msg != '':

            msg = ser.readline()
            if self.debug > 2:
                print msg

            # when we hit the end...
            if msg.count("OK") == 1:
                if finished == True:
                    if self.debug > 0:
                        print "Successful."
                    return sms_text
                else:
                    if self.debug > 0:
                        print "Read Failed. Exiting..."
                    return 0

            # when something went wrong...
            if msg.count("ERROR") == 1:
                if self.debug > 0:
                    print "Failed. Exiting..."
                return 0

            # when we hit someting like a message
            if len(msg) > 30 and msg.count("OK") == 0 and msg.count("+CMGR") == 0:
                # get rid of the stupid newlines at the end
                msg.rstrip("\n")

                #find the beginning of the body...
                header_end =  (msg.rfind("99000020F1")+24)
                length = int(msg[header_end:header_end+2],16)
                body =  msg[56:]
                if self.debug > 2:
                    print "length = %d and body = %s" % (length, body)

                # make blocks of 8 bits
                for i in range(0,len(body)/2-1):
                    b = int(body[2*i:2*i+2],16)
                    for j in range(0,8):
                        s.append(b & 1)
                        b = b >> 1

                if self.debug > 2:
                    print "Message string in 8-blocks"
                    print s

                # make blocks of 7 bits
                for i in range(0,length):
                    sept = s[7*i:7*i+7]
                    n = 0
                    for j in range(0,7):
                        n = n + (sept[j]*(2**(j)))
                    sms_text = "%s%s" % (sms_text,chr(n))
                if self.debug > 1:
                    print sms_text
                
                finished = True

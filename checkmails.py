############################################################
#                                                          #
#        Script to receive emails und interpret their      #
#        content                                           #
#                                                          #
#        Author: Stefan Wismer                             #
#                wismerst@student.ethz.ch                  #
#                                                          #
############################################################

import poplib
import string

# Debug level: 0 is nothing, more is more (max: 4)
# ATTENTION: Anything bigger than 2 will print the password!
debug = 2

# mailbox information
hostname = "pop.googlemail.com"
username = "avalontheboat"
password = "thenewcastor"

# Parsing function for the Commands
def parse(body):
    print "I'm gonna parse %s" % body

    if body.count("SetDestination"):
        if debug > 1:
            print "SetDestination-command detected!"

    elif body.count("StayWhereYouAre"):
        if debug > 1:
            print "Hold Place - Command detected!"

    elif body.count("StayOnline"):
        if debug > 1:
            print "StayOnline detected"

    else:
        if debug > 1:
            print "Syntax error in command-email. Skipping..."

# -------------------------------
# Main Function is beginning here
# -------------------------------

# Say hello nicely...
if debug > 0:
    print "\n"
    print "PROJECT AVALON - Email Receiving Script"
    print "=======================================\n\n"

    print "Email Script is running... Debug Mode is on\n"

    print "Opening connection to %s..." % hostname

# Establish connection to Google
try:
    M = poplib.POP3_SSL('pop.googlemail.com')
    M.set_debuglevel(debug-2)

except:
    if debug > 0:
        print "Failed. Exiting..."
        exit()

else:
    if debug > 0:
        print "Successful.\n"

# Log in
if debug > 0:
    print "Logging in..."

try:
    M.user(username)
    M.pass_(password)

except:
    if debug > 0:
        print "Failed. Exiting..."
        exit()

else:
    if debug > 0:
        print "Sucessful.\n"

# Retreive Inbox - Loop.
if debug > 0:
    print "Checking Mailbox..."

numMessages = len(M.list()[1])

if debug > 1:
    print "Number of mails in box is %d\n" % numMessages

i = 0
j = 0
while i < numMessages:

    if debug > 0:
        print "\nReceiving Email %d..." % (i+1)
    
    # Read Mail
    for j in M.retr(i+1)[1]:
        
        if j.count("From:"):
            sender = j    

    body =  M.retr(i+1)[1][len(M.retr(i+1)[1])-1]

    if debug > 1:
        print "Mail %d:\n%s\n%s" % ((i+1),sender,body)

    # Is ist a weather report?
    if body.count("avalon_weatherupdate"):
        if debug > 0:
            print "Weather-Report detected! Extracting data..."

    # if not: send it to the parse function which does the rest.
    parse(body)
    
    # Next!!
    i=i+1

# close Connection
if debug == 0:
    M.quit()        # quit only when on the atlantic.
                    # this will delete all messages.

if debug > 0:
    print "\nLogged out. Script finished."  

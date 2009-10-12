import poplib
import string

print "Beginning connection..."

M = poplib.POP3_SSL('pop.googlemail.com')
# M.set_debuglevel(1)
M.getwelcome()

M.user('avalontheboat')
M.pass_('thenewcastor')
numMessages = len(M.list()[1])
print "Number of mails in box is %d\n" % numMessages

if numMessages == 0:
    exit()

for i in range(numMessages):
    # scan trouh everything and print From: line
    for j in M.retr(i+1)[1]:
       
        print "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
        print j

    print "Content: %s \n" % M.retr(i+1)[1][len(M.retr(1)[1])-1]


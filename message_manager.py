# -*- coding: utf-8 -*-
'''
Created on 01.04.2009

@author: David Frey, freyd@ee.ethz.ch
'''


import socket
import select
import time

REMOTE_ADDRESS = '192.168.33.2'
#REMOTE_ADDRESS = 'localhost'
REMOTE_PORT = 2222
READ_TIMEOUT = 60 # in seconds
FAIL = 0;
OK = 1;
TRUE = 1;
FALSE = 2;
REQUEST_PENDING = 3;

TEST = False

my_socket = None

def open_connection():
    """Opens a socket and connects to remote server."""
    #create an INET, STREAMing socket
    global my_socket;
    my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    my_socket.connect((REMOTE_ADDRESS, REMOTE_PORT))


def close_connection():
    my_socket.close()

def send_data(data):
    global my_socket;
#    totalsent = 0
#    while totalsent < len(data):
#        sent = my_socket.send(data[totalsent:])
#        if sent == 0:
#            raise RuntimeError, "socket connection broken"
#        totalsent = totalsent + sent
    my_socket.sendall(data)


def end_data():
    my_socket.shutdown(socket.SHUT_WR)

def read_data():
    ret = ''
    start = time.time()
    timeout = READ_TIMEOUT
    while True:
        ready_to_read, ready_to_write, in_error = \
                   select.select(
                      [my_socket],
                      [],
                      [],
                      timeout)

        if len(ready_to_read) == 0:
            raise RuntimeError, "timeout while reading"
        read = my_socket.recv(4096)
        if len(read) == 0: break
        ret = ret + read
        timeout = READ_TIMEOUT + start - time.time()
    return ret

# below the 'public' functions
# they all throw an exception if the server returns FAIL from any command

def send_status(status):
    '''Send a status message home.

    Expects an ASCII string as single argument. Returns OK on success.'''
    if TEST:
        return True
    open_connection()
    send_data('s')
    send_data(status)
    end_data()
    answer = read_data()
    close_connection()

    code = int(answer)
    if code != OK:
        raise RuntimeError, ("illegal response from server: %d" % code)

    return code

def receive_message():
    '''Poll for a message frome home.

    Returns any message received since the last poll. If no message was received, throws an exception.'''
    if TEST:
        return 'go north'
    open_connection()
    send_data('r')
    end_data()
    answer = read_data()
    close_connection()

    code = int(answer[0])
    if code != TRUE:
        raise RuntimeError, ("illegal response from server: %d" % code)

    return answer[1:]

def read_forecast():
    '''Reads out the last weather forecast.

    Returns the data as received in the mail. If no forecast has been received yet, throws an exception.'''
    if TEST:
        return 'really nice weather'
    open_connection()
    send_data('f')
    end_data()
    answer = read_data()
    close_connection()

    code = int(answer[0])
    if code != TRUE:
        raise RuntimeError, ("illegal response from server: %d" % code)

    return answer[1:]

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        k = int(sys.argv[1])
    else:
        import random
        k = random.randint(1, 4)

    if k == 1:
        text = 'alles klar hier'
        if len(sys.argv) > 2:
            text = sys.argv[2]
        print 'sending status report...'
        answer = send_status(text)
        print 'answer is: ', answer
    elif k == 2:
        print 'receiving a message...'
        answer = receive_message('auf dem Dach vom ETF')
        print 'answer is: ', answer
    elif k == 3:
        print 'reading weather forecast...'
        answer = read_forecast()
        print 'answer is: ', answer


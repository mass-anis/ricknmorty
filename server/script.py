#! /usr/bin/python3

import socket
import sys

sock = socket.socket()
print "socket successfully created"

sock.bind(('', 3001))
sock.listen(2)

print 'listening for packets'

while True:
    connection, client_address = sock.accept()
    connection.send('packet received')

    try:
        print 'connected to: ', client_address
        while True:
            data = connection.recv(32)
            print >>sys.stderr, 'received "%s"' % data
            if data:
                print >>sys.stderr, 'Data received'
            else:
                print >>sys.stderr, 'no more data from', client_address
                break
    finally:
        connection.close()

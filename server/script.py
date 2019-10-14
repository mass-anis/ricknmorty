#! /usr/bin/python3

import socket
import sys
import vlc

player = vlc.MediaPlayer("/home/pi/REPOS/ricknmorty/server/screaming_sun.mp3")
sunsRising = vlc.MediaPlayer("/home/pi/REPOS/ricknmorty/server/suns_rising.mp3")

sock = socket.socket()

def displayMessage(message):
    print(message)
    file.write(message + '\n')
    file.flush()

file = open("/home/pi/Desktop/log.txt", "w")

displayMessage("socket successfully created")

sock.bind(('', 3001))
sock.listen(1)

displayMessage('listening for packets')
while True:
    connection, client_address = sock.accept()
    connection.send('packet received')

    try:
        sunsRising.play()
        displayMessage('connected to: ' + str(client_address))
        while True:
            data = connection.recv(32)
            displayMessage('received ' + str(data))
            if data:
                displayMessage('data received\n')
                if data == 'open':
                    player.play()
                elif data == 'close':
                    player.stop()
            else:
                displayMessage('no more data from ' + str(client_address))
                break
    finally:
        displayMessage('closing')
        connection.close()
        file.close()

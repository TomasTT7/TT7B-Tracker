#!/usr/bin/python

# Connects to DireWolf's KISS output on localhost port 8001 and logs received packets.


import socket
import datetime
import atexit


output_file = '/home/pizero1/kiss.log'


def exit_handler():
    clientsocket.shutdown(socket.SHUT_RDWR)
    clientsocket.close()


clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('localhost', 8001))
clientsocket.settimeout(0.1)


frm = False
pck = False
c = ''
packet = ''
frame = []


while True:

    try:
        c = clientsocket.recv(1)

        if ord(c) == 192: # 0xC0
            if frm:
                frm = False
                pck = False

                received = "{:%Y-%m-%d %H:%M:%S} CET".format(datetime.datetime.now())
                callsign = ''
                
                for i in range(8, 15):
                    
                    if i == 14:
                        callsign = callsign + '-'
                    
                    callsign = callsign + chr(frame[i] >> 1)

                out = received + ' ' + callsign + ' ' + packet + '\n'

                fo = open(output_file, 'a')
                fo.write(out.encode('utf8'))
                fo.close()
                
                frame[:] = []
                packet = ''
                continue
            
            else:
                frm = True
                continue

        if frm:
            frame.append(ord(c))

        if pck:
            packet = packet + c

        if ord(c) == 240 and frm == True: # 0xF0
            pck = True
            
    except:
        pass

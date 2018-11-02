# INPUT FORMAT:
# 0,1541024038,2018-10-31T22:13:58Z,OK7DMT-1,OK7DMT-1,35(11/19),0,!,OK7DMT-1,/O,49.491484,18.223109,,,1127.4,,,,"Generic, (obsolete. Digis should use APNxxx instead)",,,"XT%O,)Z%Lx,fP-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z"
#
# OUTPUT FORMAT:
# 'yyyy-mm-dd HH:MM:SS timezone sender packet\n'
# 2018-10-31 23:11:49 CET OK7DMT-1 !/5LD\S*,yON2WW^%O,)Z!Lx,fQ-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z


import math


input_file = '2018-10-31.log'
output_file = 'DireWolf_grab OK7DMT-1.txt'


def Base91_encode_2(number):
    """ Encodes a number to a two character string """
    c1 = int(number / 91)
    c2 = int(number % 91)
    return chr(c1 + 33) + chr(c2 + 33)

def Base91_encode_4(number):
    """ Encodes a number to a four character string """
    c1 = int(number / 753571)
    c2 = int(number % 753571 / 8281)
    c3 = int(number % 753571 % 8281 / 91)
    c4 = int(number % 753571 % 8281 % 91)
    return chr(c1 + 33) + chr(c2 + 33) + chr(c3 + 33) + chr(c4 + 33)


f = open(input_file, 'r')
rawinput = f.read()
f.close

packets = rawinput.split('\n')
packets.pop(0)
packets.pop(-1)

out = []

for packet in packets:
    parts = packet.split(',', 21)

    timestamp = parts[2]
    source = parts[3]
    datatype = parts[7]
    symbolcode = parts[9]
    latitude = parts[10]
    longitude = parts[11]
    altitude =  parts[14]
    comment = parts[21]

    _latitude = (90.0 - float(latitude)) * 380926.0
    _longitude = (180.0 + float(longitude)) * 190463.0
    _altitude = math.log(float(altitude) * 3.28084, 10) / math.log(1.002, 10);

    output = timestamp[0:10] + ' ' + timestamp[11:19] + ' UTC ' + source + ' ' + datatype + symbolcode[0] \
             + Base91_encode_4(_latitude) + Base91_encode_4(_longitude) + symbolcode[1] + Base91_encode_2(_altitude) + 'W' + comment[2:-1]
    out.append(output)

fo = open(output_file, 'w')

for item in out:
    fo.write((item + '\n').encode('utf-8'))
    print(item)

fo.close


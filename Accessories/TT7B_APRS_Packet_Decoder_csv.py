# DIREWOLF:     [0] OK7DMT-1>APRS,WIDE2-1:!/5LD\S*,yON2WY3%=,)ZILx,f+-D33X!!!<QU5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,
# APRS.FI:      2018-10-31 23:13:50 CET: OK7DMT-1>APRS,WIDE2-1,qAO,OK9STS:!/5LD\S*,yON2WXT%O,)Z%Lx,fP-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
# PROCESSED:    2018-10-31 23:13:50 CET OK7DMT-1 !/5LD\S*,yON2WXT%O,)Z%Lx,fP-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
#
# Inputs a file with processed packets from aprs.fi and outputs a CSV file of decoded telemetry.


from datetime import datetime, timedelta
import math


input_file = 'APRSfi_grab OK7DMT-1.txt'
utc_offset = 1


def Base91_2(string):
    """ Decode a 2 symbol Base91 code """
    num = (ord(string[0]) - 33) * 91 + (ord(string[1]) - 33)
    return num

def Base91_3(string):
    """ Decode a 3 symbol Base91 code """
    num = (ord(string[0]) - 33) * 8281 + (ord(string[1]) - 33) * 91 + (ord(string[2]) - 33)
    return num

def Base91_4(string):
    """ Decode a 4 symbol Base91 code """
    num = (ord(string[0]) - 33) * 753571 + (ord(string[1]) - 33) * 8281 \
        + (ord(string[2]) - 33) * 91 + (ord(string[3]) - 33)
    return num

def Base91_5(string):
    """ Decode a 5 symbol Base91 code """
    num = (ord(string[0]) - 33) * 68574961 + (ord(string[1]) - 33) * 753571 + (ord(string[2]) - 33) * 8281 \
        + (ord(string[3]) - 33) * 91 + (ord(string[4]) - 33)
    return num


year, month, day, hour, minute, second, callsign, latitude, longitude, altitude = ([] for i in range(10))
tempMCU, tempTH1, tempTH2, tempMS1, tempMS2, presMS1, presMS2, battV, light = ([] for i in range(9))


f = open(input_file, 'r')
rawinput = f.read()
f.close

packets = rawinput.split('\n')
packets.pop(-1)

for packet in packets:
    parts = packet.split(' ', 4)
    telemetry = parts[4][0:38]
    backlog = parts[4][38:75]

    # Receiver Date & Time UTC
    d = datetime.strptime(parts[0] + ' ' + parts[1], '%Y-%m-%d %H:%M:%S')
    d = d - timedelta(hours=utc_offset)

    year.append(d.year)
    month.append(d.month)
    day.append(d.day)
    hour.append(d.hour)
    minute.append(d.minute)
    second.append(d.second)

    # Transmitter Callsign
    callsign.append(parts[3])

    # Telemetry
    lat = Base91_4(telemetry[2:6])
    lat = 90.0 - lat / 380926.0
    latitude.append(lat)

    lon = Base91_4(telemetry[6:10])
    lon = lon / 190463.0 - 180.0
    longitude.append(lon)

    alt = Base91_2(telemetry[11:13])
    alt = 10.0**(math.log(1.002) / math.log(10) * alt) * 0.3048
    altitude.append(alt)
    
    tMCU = Base91_2(telemetry[14:16])
    tMCU = (tMCU - 4000.0) / 50.0
    tempMCU.append(tMCU)

    tTH1 = Base91_2(telemetry[16:18])
    tTH1 = 1 / (0.00128424 + 0.00023629 * math.log((tTH1 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH1 / 4095.0 * 1.826))) \
         + 0.0000000928 * math.log((tTH1 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH1 / 4095.0 * 1.826)))**3) - 273.15
    tempTH1.append(tTH1)

    tTH2 = Base91_2(telemetry[18:20])
    tTH2 = 1 / (0.00128424 + 0.00023629 * math.log((tTH2 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH2 / 4095.0 * 1.826))) \
         + 0.0000000928 * math.log((tTH2 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH2 / 4095.0 * 1.826)))**3) - 273.15
    tempTH2.append(tTH2)

    tMS1 = Base91_2(telemetry[20:22])
    tMS1 = (tMS1 - 4000.0) / 50.0
    tempMS1.append(tMS1)

    tMS2 = Base91_2(telemetry[22:24])
    tMS2 = (tMS2 - 4000.0) / 50.0
    tempMS2.append(tMS2)

    presMS1.append(Base91_3(telemetry[24:27]))
            
    presMS2.append(Base91_3(telemetry[27:30]))

    bV = Base91_2(telemetry[30:32])
    bV = bV / 4095.0 * 1.826 / 0.5
    battV.append(bV)

    lit = Base91_2(telemetry[32:34])
    lit = 10.0**(math.log(1.002) / math.log(10) * lit) / 139.0
    light.append(lit)
    
    # Backlog


# CSV
#f1= open('TT7F5data.csv', 'w')
#f1.write("""Timestamp,Solar,Supercap,MCU,TX,Satellites,Sequence,Img,PSM,Altitude\n""")

#for i in range(len(dataORDERED)):
    
#    dT = dataORDERED[i].split(' ')
#    f1.write("""{:s}-{:s}-{:s} {:s}:{:s}:{:s},{:s},{:s},{:s},{:s},{:s},{:s},{:s},{:s},{:s}\n""".format(dT[1], dT[2], dT[3], dT[4], dT[5], dT[6], dT[11], dT[12], dT[13], dT[14], dT[15], dT[10], dT[18], dT[19], dT[9]))
#
#f1.close()













# DIREWOLF:     [0] OK7DMT-1>APRS,WIDE2-1:!/5LD\S*,yON2WY3%=,)ZILx,f+-D33X!!!<QU5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,
# APRS.FI:      2018-10-31 23:13:50 CET: OK7DMT-1>APRS,WIDE2-1,qAO,OK9STS:!/5LD\S*,yON2WXT%O,)Z%Lx,fP-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
# PROCESSED:    2018-10-31 23:13:50 CET OK7DMT-1 !/5LD\S*,yON2WXT%O,)Z%Lx,fP-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
#
# Inputs a file with processed packets from aprs.fi and outputs a CSV file of decoded telemetry.

# -*- coding: cp1250 -*-

from datetime import datetime, timedelta
import math


input_file = 'APRSfi_grab OK7DMT-1.txt'
#input_file = 'DireWolf_grab OK7DMT-1.txt'
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


last_reset = {0: "NONE",
              1: "POR",
              2: "BOD",
              3: "EXT",
              4: "WDT",
              5: "SYS"}

year, month, day, hour, minute, second, callsign, latitude, longitude, altitude = ([] for i in range(10))
tempMCU, tempTH1, tempTH2, tempMS1, tempMS2, presMS1, presMS2, battV, light, altitude_offset = ([] for i in range(10))
satellites, time_active, reset, latitudeB, longitudeB, altitude_preciseB, satellitesB, resetB, yearB, monthB = ([] for i in range(10))
dayB, hourB, minuteB, time_activeB, tempMCUB, tempTH1B, tempTH2B, tempMS1B, tempMS2B, presMS1B = ([] for i in range(10))
presMS2B, battVB, lightB, callsignB = ([] for i in range(4))

f = open(input_file, 'r')
rawinput = f.read()
f.close

packets = rawinput.split('\n')
packets.pop(-1)

for packet in packets:
    parts = packet.split(' ', 4)
    telemetry = parts[4][0:38]
    try:
        backlog = parts[4][38:75]
    except:
        backlog = ''

    # Receiver Date & Time UTC
    d = datetime.strptime(parts[0] + ' ' + parts[1], '%Y-%m-%d %H:%M:%S')
    
    if parts[2] != 'UTC':
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
    alt = 1.002**(alt) * 0.3048
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

    data1 = Base91_4(telemetry[34:38])
    altitude_offset.append(int(data1 / 6 / 1000 / 17))
    satellites.append(int(data1 / 6 / 1000 % 17))
    time_active.append(int(data1 / 6 % 1000))
    reset.append(data1 % 6)
    
    # Backlog
    if backlog is not '':
        callsignB.append(callsign[-1])
        
        latB = Base91_4(backlog[0:4])
        latB = 90.0 - latB / 380926.0
        latitudeB.append(latB)

        lonB = Base91_4(backlog[4:8])
        lonB = lonB / 190463.0 - 180.0
        longitudeB.append(lonB)

        data2 = Base91_4(backlog[8:12])
        altitude_preciseB.append(int(data2 / 6 / 17))
        satellitesB.append(int(data2 / 6 % 17))
        resetB.append(data2 % 6)

        data3 = Base91_5(backlog[12:17])
        yearB.append(int(data3 / 1000 / 60 / 24 / 31 / 12) + 2018)
        monthB.append(int(data3 / 1000 / 60 / 24 / 31 % 12) + 1)
        dayB.append(int(data3 / 1000 / 60 / 24 % 31) + 1)
        hourB.append(int(data3 / 1000 / 60 % 24))
        minuteB.append(int(data3 / 1000 % 60))
        time_activeB.append(data3 % 1000)

        tMCUB = Base91_2(backlog[17:19])
        tMCUB = (tMCUB - 4000.0) / 50.0
        tempMCUB.append(tMCUB)

        tTH1B = Base91_2(backlog[19:21])
        tTH1B = 1 / (0.00128424 + 0.00023629 * math.log((tTH1B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH1B / 4095.0 * 1.826))) \
              + 0.0000000928 * math.log((tTH1B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH1B / 4095.0 * 1.826)))**3) - 273.15
        tempTH1B.append(tTH1B)

        tTH2B = Base91_2(backlog[21:23])
        tTH2B = 1 / (0.00128424 + 0.00023629 * math.log((tTH2B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH2B / 4095.0 * 1.826))) \
             + 0.0000000928 * math.log((tTH2B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tTH2B / 4095.0 * 1.826)))**3) - 273.15
        tempTH2B.append(tTH2B)

        tMS1B = Base91_2(backlog[23:25])
        tMS1B = (tMS1B - 4000.0) / 50.0
        tempMS1B.append(tMS1B)

        tMS2B = Base91_2(backlog[25:27])
        tMS2B = (tMS2B - 4000.0) / 50.0
        tempMS2B.append(tMS2B)

        presMS1B.append(Base91_3(backlog[27:30]))

        presMS2B.append(Base91_3(backlog[30:33]))

        bVB = Base91_2(backlog[33:35])
        bVB = bVB / 4095.0 * 1.826 / 0.5
        battVB.append(bVB)

        litB = Base91_2(backlog[35:37])
        litB = 10.0**(math.log(1.002) / math.log(10) * litB) / 139.0
        lightB.append(litB)

# CSV
header = """Timestamp,Callsign,Latitude,Longitude,Altitude (coarse),Altitude (precise),Altitude (offset),Temperature MCU,Temperature TH1,Temperature TH2,Temperature MS1,Temperature MS2,Pressure MS1,Pressure MS2,Battery Voltage,Ambient Light,Satellites,Active Time,Last Reset,Source"""

units = """[UTC],,[°],[°],[m],[m],[m],[°C],[°C],[°C],[°C],[°C],[Pa],[Pa],[V],[lux],[n],[s],,[L/B]"""

output = []

for i in range(len(year)):
    output.append("""{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d},{:s},{:.5f},{:.5f},{:.1f},{:d},{:d},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f},{:d},{:d},{:.3f},{:.4f},{:d},{:.1f},{:s},L"""
                  .format(year[i], month[i], day[i], hour[i], minute[i], second[i], callsign[i], latitude[i], longitude[i],
                          altitude[i], int(altitude[i]) + altitude_offset[i], altitude_offset[i], tempMCU[i], tempTH1[i],
                          tempTH2[i], tempMS1[i], tempMS2[i], presMS1[i], presMS2[i], battV[i], light[i], satellites[i],
                          time_active[i] / 10.0, last_reset[reset[i]]))

for i in range(len(yearB)):
    output.append("""{:04d}-{:02d}-{:02d} {:02d}:{:02d}:00,{:s},{:.5f},{:.5f},{:.1f},{:d},{:d},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f},{:d},{:d},{:.3f},{:.4f},{:d},{:.1f},{:s},B"""
                  .format(yearB[i], monthB[i], dayB[i], hourB[i], minuteB[i], callsignB[i], latitudeB[i], longitudeB[i],
                          altitude_preciseB[i], altitude_preciseB[i], 0, tempMCUB[i], tempTH1B[i], tempTH2B[i], tempMS1B[i],
                          tempMS2B[i], presMS1B[i], presMS2B[i], battVB[i], lightB[i], satellitesB[i], time_activeB[i] / 10.0,
                          last_reset[resetB[i]]))

final = []
output_seen = set()

for pkt in output:
    if pkt not in output_seen:
        final.append(pkt)
        output_seen.add(pkt)

final.sort()

fo = open(callsign[0] + ' data.csv', 'w')
fo.write(header + '\n')
fo.write(units + '\n')

for pkt in final:
    fo.write(pkt + '\n')
    print(pkt)

fo.close()













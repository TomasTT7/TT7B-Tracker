# parses list data output from http://weather.uwyo.edu/upperair/sounding.html
# returns altitudes/pressures/temperatures of a specific density level


density = 0.2664       # select density level
input_list = ['belem BR 201812.txt', 'belem BR 201901.txt', 'belem BR 201902.txt', 'belem BR 201903.txt', 'belem BR 201904.txt', 'belem BR 201905.txt', 'belem BR 201906.txt', 'belem BR 201907.txt', 'belem BR 201908.txt', 'belem BR 201909.txt', 'belem BR 201910.txt', 'belem BR 201911.txt']


month = {'Jan':'01', 'Feb':'02', 'Mar':'03', 'Apr':'04', 'May':'05', 'Jun':'06', 'Jul':'07', 'Aug':'08', 'Sep':'09', 'Oct':'10', 'Nov':'11', 'Dec':'12'}


for n in range(len(input_list)):
    
    f = open(input_list[n], 'r')
    rawdata = f.read()
    f.close

    soundings = rawdata.split('Observations at')

    for i in range(1, len(soundings)):

        datablock = soundings[i].split('Station information and sounding indices')
        
        infolines = datablock[1].split('\n')
        splitlat = infolines[5].split(':')
        splitlon = infolines[6].split(':')
        
        datalines = datablock[0].split('\n')
        datafields = datalines[0].split(' ')

        timestamp = datafields[4] + '-' + month[datafields[3]] + '-' + datafields[2] + ' ' + datafields[1][0:2] + ':00'
        latitude = float(splitlat[1])
        longitude = float(splitlon[1])

        _pressure_prev = 0.0
        _altitude_prev = 0.0
        _temperature_prev = 0.0
        _density_prev = 0.0

        for line in range(6, len(datalines)-2):

            datapoints = datalines[line].split(' ')

            if len(datapoints) < 10:
                continue
            
            filtered = filter(None, datapoints[0:10])
            
            if len(filtered) < 3:
                continue
            
            _pressure = float(filtered[0])
            _altitude = float(filtered[1])
            _temperature = float(filtered[2])
            _density = (float(_pressure) * 100.0) / (287.05 * (float(_temperature) + 273.15))

            if _density < density:
                altitude = (_density_prev - density) / (_density_prev - _density) * (_altitude - _altitude_prev) + _altitude_prev
                pressure = _pressure_prev - (_density_prev - density) / (_density_prev - _density) * (_pressure_prev - _pressure)
                temperature = (_density_prev - density) / (_density_prev - _density) * (_temperature - _temperature_prev) + _temperature_prev

                print("Station,{:.4f},{:.4f},{:s},{:.5f},{:.0f},{:.0f},{:.1f}".format(latitude, longitude, timestamp, density, altitude, pressure * 100.0, temperature))
                break

            _pressure_prev = _pressure
            _altitude_prev = _altitude
            _temperature_prev = _temperature
            _density_prev = _density


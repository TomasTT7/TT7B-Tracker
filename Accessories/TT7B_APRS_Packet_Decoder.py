# EXAMPLE:
# !/5LD\S*,yON2WYm%=,)ZiLx,f:-D33ZM0!<QU
# !/5LD\S*,yON2WYm%=,)ZiLx,f:-D33ZM0!<QU5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z

from Tkinter import *
import ttk
import math


reset = {0: "none",
         1: "POR",
         2: "BOD",
         3: "EXT",
         4: "WDT",
         5: "SYS"}


class Application(Frame):

    def __init__(self, master):
        """ Initialize the Frame """
        Frame.__init__(self, master)
        self.grid()
        self.create_widgets()

    def create_widgets(self):
        """ Create several widgets """
        self.port_label = Label(self, text="Input:")
        self.port_label.grid(row=0, column=0, padx=5, sticky=W)
        self.port_label_1 = Label(self, text="__________")
        self.port_label_1.grid(row=42, column=0, sticky=W, pady=5)
        self.port_label_2 = Label(self, text="_____________________________")
        self.port_label_2.grid(row=42, column=1, sticky=W)
        self.port_label_3 = Label(self, text="_______________")
        self.port_label_3.grid(row=42, column=2, sticky=W)
        self.port_label_4 = Label(self, text="_______________")
        self.port_label_4.grid(row=42, column=3, sticky=W)
        self.port_label_5 = Label(self, text="________")
        self.port_label_5.grid(row=42, column=4, sticky=W)

        self.packet_in = Entry(self)
        self.packet_in.grid(row=1, column=0, sticky=W+E, padx=5, columnspan=5)

        self.decode_button = Button(self, text="Decode", command=self.decode)
        self.decode_button.grid(row=0, column=4, sticky=W, pady=5)

        self.out_label = Label(self, text="MAIN")
        self.out_label.grid(row=3, column=0, sticky=W, padx=5, columnspan=2)

        self.latitude_label = Label(self, text="Latitude:")
        self.latitude_label.grid(row=5, column=1, sticky=W)
        self.latitude_value = Label(self, text="")
        self.latitude_value.grid(row=5, column=2, sticky=W)
        self.latitude_value2 = Label(self, text="")
        self.latitude_value2.grid(row=5, column=3, sticky=W)

        self.longitude_label = Label(self, text="Longitude:")
        self.longitude_label.grid(row=6, column=1, sticky=W)
        self.longitude_value = Label(self, text="")
        self.longitude_value.grid(row=6, column=2, sticky=W)
        self.longitude_value2 = Label(self, text="")
        self.longitude_value2.grid(row=6, column=3, sticky=W)

        self.altitude_label = Label(self, text="Altitude coarse:")
        self.altitude_label.grid(row=7, column=1, sticky=W)
        self.altitude_value = Label(self, text="")
        self.altitude_value.grid(row=7, column=2, sticky=W)
        self.altitude_value2 = Label(self, text="")
        self.altitude_value2.grid(row=7, column=3, sticky=W)

        self.tempMCU_label = Label(self, text="Temperature MCU:")
        self.tempMCU_label.grid(row=8, column=1, sticky=W)
        self.tempMCU_value = Label(self, text="")
        self.tempMCU_value.grid(row=8, column=2, sticky=W)
        self.tempMCU_value2 = Label(self, text="")
        self.tempMCU_value2.grid(row=8, column=3, sticky=W)

        self.tempTH1_label = Label(self, text="Temperature THERMISTOR_1:")
        self.tempTH1_label.grid(row=9, column=1, sticky=W)
        self.tempTH1_value = Label(self, text="")
        self.tempTH1_value.grid(row=9, column=2, sticky=W)
        self.tempTH1_value2 = Label(self, text="")
        self.tempTH1_value2.grid(row=9, column=3, sticky=W)

        self.tempTH2_label = Label(self, text="Temperature THERMISTOR_2:")
        self.tempTH2_label.grid(row=10, column=1, sticky=W)
        self.tempTH2_value = Label(self, text="")
        self.tempTH2_value.grid(row=10, column=2, sticky=W)
        self.tempTH2_value2 = Label(self, text="")
        self.tempTH2_value2.grid(row=10, column=3, sticky=W)

        self.tempMS1_label = Label(self, text="Temperature MS5607_1:")
        self.tempMS1_label.grid(row=11, column=1, sticky=W)
        self.tempMS1_value = Label(self, text="")
        self.tempMS1_value.grid(row=11, column=2, sticky=W)
        self.tempMS1_value2 = Label(self, text="")
        self.tempMS1_value2.grid(row=11, column=3, sticky=W)

        self.tempMS2_label = Label(self, text="Temperature MS5607_2:")
        self.tempMS2_label.grid(row=12, column=1, sticky=W)
        self.tempMS2_value = Label(self, text="")
        self.tempMS2_value.grid(row=12, column=2, sticky=W)
        self.tempMS2_value2 = Label(self, text="")
        self.tempMS2_value2.grid(row=12, column=3, sticky=W)

        self.presMS1_label = Label(self, text="Pressure MS5607_1:")
        self.presMS1_label.grid(row=13, column=1, sticky=W)
        self.presMS1_value = Label(self, text="")
        self.presMS1_value.grid(row=13, column=2, sticky=W)
        self.presMS1_value2 = Label(self, text="")
        self.presMS1_value2.grid(row=13, column=3, sticky=W)

        self.presMS2_label = Label(self, text="Pressure MS5607_2:")
        self.presMS2_label.grid(row=14, column=1, sticky=W)
        self.presMS2_value = Label(self, text="")
        self.presMS2_value.grid(row=14, column=2, sticky=W)
        self.presMS2_value2 = Label(self, text="")
        self.presMS2_value2.grid(row=14, column=3, sticky=W)

        self.battV_label = Label(self, text="Battery Voltage:")
        self.battV_label.grid(row=15, column=1, sticky=W)
        self.battV_value = Label(self, text="")
        self.battV_value.grid(row=15, column=2, sticky=W)
        self.battV_value2 = Label(self, text="")
        self.battV_value2.grid(row=15, column=3, sticky=W)

        self.light_label = Label(self, text="Ambient Light:")
        self.light_label.grid(row=16, column=1, sticky=W)
        self.light_value = Label(self, text="")
        self.light_value.grid(row=16, column=2, sticky=W)
        self.light_value2 = Label(self, text="")
        self.light_value2.grid(row=16, column=3, sticky=W)

        self.altitudeP_label = Label(self, text="Altitude precise:")
        self.altitudeP_label.grid(row=17, column=1, sticky=W)
        self.altitudeP_value = Label(self, text="")
        self.altitudeP_value.grid(row=17, column=2, sticky=W)

        self.data1_value2 = Label(self, text="")
        self.data1_value2.grid(row=17, column=3, sticky=W)

        self.sats_label = Label(self, text="Satellites:")
        self.sats_label.grid(row=18, column=1, sticky=W)
        self.sats_value = Label(self, text="")
        self.sats_value.grid(row=18, column=2, sticky=W)

        self.time_label = Label(self, text="Active Time:")
        self.time_label.grid(row=19, column=1, sticky=W)
        self.time_value = Label(self, text="")
        self.time_value.grid(row=19, column=2, sticky=W)

        self.res_label = Label(self, text="Last Reset:")
        self.res_label.grid(row=20, column=1, sticky=W)
        self.res_value = Label(self, text="")
        self.res_value.grid(row=20, column=2, sticky=W)

        self.out_label2 = Label(self, text="BACKLOG")
        self.out_label2.grid(row=21, column=0, sticky=W, padx=5)
        self.out_label2_2 = Label(self, text="")
        self.out_label2_2.grid(row=21, column=2, sticky=W, columnspan=2)

        self.latitudeB_label = Label(self, text="Latitude:")
        self.latitudeB_label.grid(row=22, column=1, sticky=W)
        self.latitudeB_value = Label(self, text="")
        self.latitudeB_value.grid(row=22, column=2, sticky=W)
        self.latitudeB_value2 = Label(self, text="")
        self.latitudeB_value2.grid(row=22, column=3, sticky=W)

        self.longitudeB_label = Label(self, text="Longitude:")
        self.longitudeB_label.grid(row=23, column=1, sticky=W)
        self.longitudeB_value = Label(self, text="")
        self.longitudeB_value.grid(row=23, column=2, sticky=W)
        self.longitudeB_value2 = Label(self, text="")
        self.longitudeB_value2.grid(row=23, column=3, sticky=W)

        self.altitudePB_label = Label(self, text="Altitude precise:")
        self.altitudePB_label.grid(row=24, column=1, sticky=W)
        self.altitudePB_value = Label(self, text="")
        self.altitudePB_value.grid(row=24, column=2, sticky=W)

        self.data2_value2 = Label(self, text="")
        self.data2_value2.grid(row=24, column=3, sticky=W)

        self.satsB_label = Label(self, text="Satellites:")
        self.satsB_label.grid(row=25, column=1, sticky=W)
        self.satsB_value = Label(self, text="")
        self.satsB_value.grid(row=25, column=2, sticky=W)
        
        self.resB_label = Label(self, text="Last Reset:")
        self.resB_label.grid(row=26, column=1, sticky=W)
        self.resB_value = Label(self, text="")
        self.resB_value.grid(row=26, column=2, sticky=W)

        self.yearB_label = Label(self, text="Year:")
        self.yearB_label.grid(row=27, column=1, sticky=W)
        self.yearB_value = Label(self, text="")
        self.yearB_value.grid(row=27, column=2, sticky=W)

        self.data3_value2 = Label(self, text="")
        self.data3_value2.grid(row=27, column=3, sticky=W)

        self.monthB_label = Label(self, text="Month:")
        self.monthB_label.grid(row=28, column=1, sticky=W)
        self.monthB_value = Label(self, text="")
        self.monthB_value.grid(row=28, column=2, sticky=W)

        self.dayB_label = Label(self, text="Day:")
        self.dayB_label.grid(row=29, column=1, sticky=W)
        self.dayB_value = Label(self, text="")
        self.dayB_value.grid(row=29, column=2, sticky=W)

        self.hourB_label = Label(self, text="Hour:")
        self.hourB_label.grid(row=30, column=1, sticky=W)
        self.hourB_value = Label(self, text="")
        self.hourB_value.grid(row=30, column=2, sticky=W)

        self.minuteB_label = Label(self, text="Minute:")
        self.minuteB_label.grid(row=31, column=1, sticky=W)
        self.minuteB_value = Label(self, text="")
        self.minuteB_value.grid(row=31, column=2, sticky=W)

        self.timeB_label = Label(self, text="Active Time:")
        self.timeB_label.grid(row=32, column=1, sticky=W)
        self.timeB_value = Label(self, text="")
        self.timeB_value.grid(row=32, column=2, sticky=W)

        self.tempMCUB_label = Label(self, text="Temperature MCU:")
        self.tempMCUB_label.grid(row=33, column=1, sticky=W)
        self.tempMCUB_value = Label(self, text="")
        self.tempMCUB_value.grid(row=33, column=2, sticky=W)
        self.tempMCUB_value2 = Label(self, text="")
        self.tempMCUB_value2.grid(row=33, column=3, sticky=W)

        self.tempTH1B_label = Label(self, text="Temperature THERMISTOR_1:")
        self.tempTH1B_label.grid(row=34, column=1, sticky=W)
        self.tempTH1B_value = Label(self, text="")
        self.tempTH1B_value.grid(row=34, column=2, sticky=W)
        self.tempTH1B_value2 = Label(self, text="")
        self.tempTH1B_value2.grid(row=34, column=3, sticky=W)

        self.tempTH2B_label = Label(self, text="Temperature THERMISTOR_2:")
        self.tempTH2B_label.grid(row=35, column=1, sticky=W)
        self.tempTH2B_value = Label(self, text="")
        self.tempTH2B_value.grid(row=35, column=2, sticky=W)
        self.tempTH2B_value2 = Label(self, text="")
        self.tempTH2B_value2.grid(row=35, column=3, sticky=W)

        self.tempMS1B_label = Label(self, text="Temperature MS5607_1:")
        self.tempMS1B_label.grid(row=36, column=1, sticky=W)
        self.tempMS1B_value = Label(self, text="")
        self.tempMS1B_value.grid(row=36, column=2, sticky=W)
        self.tempMS1B_value2 = Label(self, text="")
        self.tempMS1B_value2.grid(row=36, column=3, sticky=W)

        self.tempMS2B_label = Label(self, text="Temperature MS5607_2:")
        self.tempMS2B_label.grid(row=37, column=1, sticky=W)
        self.tempMS2B_value = Label(self, text="")
        self.tempMS2B_value.grid(row=37, column=2, sticky=W)
        self.tempMS2B_value2 = Label(self, text="")
        self.tempMS2B_value2.grid(row=37, column=3, sticky=W)

        self.presMS1B_label = Label(self, text="Pressure MS5607_1:")
        self.presMS1B_label.grid(row=38, column=1, sticky=W)
        self.presMS1B_value = Label(self, text="")
        self.presMS1B_value.grid(row=38, column=2, sticky=W)
        self.presMS1B_value2 = Label(self, text="")
        self.presMS1B_value2.grid(row=38, column=3, sticky=W)

        self.presMS2B_label = Label(self, text="Pressure MS5607_2:")
        self.presMS2B_label.grid(row=39, column=1, sticky=W)
        self.presMS2B_value = Label(self, text="")
        self.presMS2B_value.grid(row=39, column=2, sticky=W)
        self.presMS2B_value2 = Label(self, text="")
        self.presMS2B_value2.grid(row=39, column=3, sticky=W)

        self.battVB_label = Label(self, text="Battery Voltage:")
        self.battVB_label.grid(row=40, column=1, sticky=W)
        self.battVB_value = Label(self, text="")
        self.battVB_value.grid(row=40, column=2, sticky=W)
        self.battVB_value2 = Label(self, text="")
        self.battVB_value2.grid(row=40, column=3, sticky=W)
        
        self.lightB_label = Label(self, text="Ambient Light:")
        self.lightB_label.grid(row=41, column=1, sticky=W)
        self.lightB_value = Label(self, text="")
        self.lightB_value.grid(row=41, column=2, sticky=W)
        self.lightB_value2 = Label(self, text="")
        self.lightB_value2.grid(row=41, column=3, sticky=W)

        self.out_label3 = Label(self, text="")
        self.out_label3.grid(row=2, column=0, sticky=W, padx=5, pady=5, columnspan=5)

    def decode(self):
        """ Check and decode an APRS packet """
        content = self.packet_in.get()
        
        if len(content) is 0:
            self.out_label3.config(text="Not a valid packet!")
            return

        if  content[0] is '!' and (len(content) is 38 or len(content) is 75):
            self.out_label3.config(text=content)

            latitude = self.Base91_4(content[2:6])
            latitude = 90.0 - latitude / 380926.0
            self.latitude_value.config(text='{0:.5f}'.format(latitude) + u"\u00b0")
            self.latitude_value2.config(text=content[2:6])

            longitude = self.Base91_4(content[6:10])
            longitude = longitude / 190463.0 - 180.0
            self.longitude_value.config(text='{0:.5f}'.format(longitude) + u"\u00b0")
            self.longitude_value2.config(text=content[6:10])
            
            altitude = self.Base91_2(content[11:13])
            altitude = 10.0**(math.log(1.002) / math.log(10) * altitude) * 0.3048
            self.altitude_value.config(text='{0:.0f}'.format(altitude) + "m")
            self.altitude_value2.config(text=content[11:13])

            tempMCU = self.Base91_2(content[14:16])
            tempMCU = (tempMCU - 4000.0) / 50.0
            self.tempMCU_value.config(text='{0:.2f}'.format(tempMCU) + u"\u2103")
            self.tempMCU_value2.config(text=content[14:16])

            tempTH1 = self.Base91_2(content[16:18])
            tempTH1 = 1 / (0.00128424 + 0.00023629 * math.log((tempTH1 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH1 / 4095.0 * 1.826))) \
                    + 0.0000000928 * math.log((tempTH1 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH1 / 4095.0 * 1.826)))**3) - 273.15
            self.tempTH1_value.config(text='{0:.2f}'.format(tempTH1) + u"\u2103")
            self.tempTH1_value2.config(text=content[16:18])

            tempTH2 = self.Base91_2(content[18:20])
            tempTH2 = 1 / (0.00128424 + 0.00023629 * math.log((tempTH2 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH2 / 4095.0 * 1.826))) \
                    + 0.0000000928 * math.log((tempTH2 / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH2 / 4095.0 * 1.826)))**3) - 273.15
            self.tempTH2_value.config(text='{0:.2f}'.format(tempTH2) + u"\u2103")
            self.tempTH2_value2.config(text=content[18:20])

            tempMS1 = self.Base91_2(content[20:22])
            tempMS1 = (tempMS1 - 4000.0) / 50.0
            self.tempMS1_value.config(text='{0:.2f}'.format(tempMS1) + u"\u2103")
            self.tempMS1_value2.config(text=content[20:22])

            tempMS2 = self.Base91_2(content[22:24])
            tempMS2 = (tempMS2 - 4000.0) / 50.0
            self.tempMS2_value.config(text='{0:.2f}'.format(tempMS2) + u"\u2103")
            self.tempMS2_value2.config(text=content[22:24])

            presMS1 = self.Base91_3(content[24:27])
            self.presMS1_value.config(text='{0:.0f}'.format(presMS1) + "Pa")
            self.presMS1_value2.config(text=content[24:27])
            
            presMS2 = self.Base91_3(content[27:30])
            self.presMS2_value.config(text='{0:.0f}'.format(presMS2) + "Pa")
            self.presMS2_value2.config(text=content[27:30])

            battV = self.Base91_2(content[30:32])
            battV = battV / 4095.0 * 1.826 / 0.5
            self.battV_value.config(text='{0:.3f}'.format(battV) + "V")
            self.battV_value2.config(text=content[30:32])

            light = self.Base91_2(content[32:34])
            light = 10.0**(math.log(1.002) / math.log(10) * light) / 139.0
            if light < 1.0:
                self.light_value.config(text='{0:.4f}'.format(light) + "lux")
            elif light < 10.0:
                self.light_value.config(text='{0:.3f}'.format(light) + "lux")
            elif light < 100.0:
                self.light_value.config(text='{0:.2f}'.format(light) + "lux")
            else:
                self.light_value.config(text='{0:.0f}'.format(light) + "lux")
            self.light_value2.config(text=content[32:34])

            data1 = self.Base91_4(content[34:38])
            altitudeP = int(data1 / 6 / 1000 / 17)
            sats = int(data1 / 6 / 1000 % 17)
            time = int(data1 / 6 % 1000)
            res = data1 % 6
            self.altitudeP_value.config(text='{0:.0f}'.format(altitudeP) + "m" + " (" + str(altitudeP + int(altitude)) + "m)")
            self.sats_value.config(text='{0:.0f}'.format(sats))
            self.time_value.config(text='{0:.1f}'.format(time / 10.0) + "s")
            self.res_value.config(text=reset[res])
            self.data1_value2.config(text=content[34:38])

            if len(content) is 75:
                latitudeB = self.Base91_4(content[38:42])
                latitudeB = 90.0 - latitudeB / 380926.0
                self.latitudeB_value.config(text='{0:.5f}'.format(latitudeB) + u"\u00b0")
                self.latitudeB_value2.config(text=content[38:42])

                longitudeB = self.Base91_4(content[42:46])
                longitudeB = longitudeB / 190463.0 - 180.0
                self.longitudeB_value.config(text='{0:.5f}'.format(longitudeB) + u"\u00b0")
                self.longitudeB_value2.config(text=content[42:46])

                data2 = self.Base91_4(content[46:50])
                altitudePB = int(data2 / 6 / 17)
                satsB = int(data2 / 6 % 17)
                resB = data2 % 6
                self.altitudePB_value.config(text='{0:.0f}'.format(altitudePB) + "m")
                self.satsB_value.config(text='{0:.0f}'.format(satsB))
                self.resB_value.config(text=reset[resB])
                self.data2_value2.config(text=content[46:50])

                data3 = self.Base91_5(content[50:55])
                yearB = int(data3 / 1000 / 60 / 24 / 31 / 12) + 2018
                monthB = int(data3 / 1000 / 60 / 24 / 31 % 12) + 1
                dayB = int(data3 / 1000 / 60 / 24 % 31) + 1
                hourB = int(data3 / 1000 / 60 % 24)
                minuteB = int(data3 / 1000 % 60)
                timeB = data3 % 1000
                self.yearB_value.config(text='{0:.0f}'.format(yearB))
                self.monthB_value.config(text='{0:.0f}'.format(monthB))
                self.dayB_value.config(text='{0:.0f}'.format(dayB))
                self.hourB_value.config(text='{0:.0f}'.format(hourB) + "h")
                self.minuteB_value.config(text='{0:.0f}'.format(minuteB) + "min")
                self.timeB_value.config(text='{0:.1f}'.format(timeB / 10.0) + "s")
                self.data3_value2.config(text=content[50:55])

                tempMCUB = self.Base91_2(content[55:57])
                tempMCUB = (tempMCUB - 4000.0) / 50.0
                self.tempMCUB_value.config(text='{0:.2f}'.format(tempMCUB) + u"\u2103")
                self.tempMCUB_value2.config(text=content[55:57])

                tempTH1B = self.Base91_2(content[57:59])
                tempTH1B = 1 / (0.00128424 + 0.00023629 * math.log((tempTH1B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH1B / 4095.0 * 1.826))) \
                        + 0.0000000928 * math.log((tempTH1B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH1B / 4095.0 * 1.826)))**3) - 273.15
                self.tempTH1B_value.config(text='{0:.2f}'.format(tempTH1B) + u"\u2103")
                self.tempTH1B_value2.config(text=content[57:59])

                tempTH2B = self.Base91_2(content[59:61])
                tempTH2B = 1 / (0.00128424 + 0.00023629 * math.log((tempTH2B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH2B / 4095.0 * 1.826))) \
                        + 0.0000000928 * math.log((tempTH2B / 4095.0 * 1.826) * 49900.0 / (1.826 - (tempTH2B / 4095.0 * 1.826)))**3) - 273.15
                self.tempTH2B_value.config(text='{0:.2f}'.format(tempTH2B) + u"\u2103")
                self.tempTH2B_value2.config(text=content[59:61])

                tempMS1B = self.Base91_2(content[61:63])
                tempMS1B = (tempMS1B - 4000.0) / 50.0
                self.tempMS1B_value.config(text='{0:.2f}'.format(tempMS1B) + u"\u2103")
                self.tempMS1B_value2.config(text=content[61:63])

                tempMS2B = self.Base91_2(content[63:65])
                tempMS2B = (tempMS2B - 4000.0) / 50.0
                self.tempMS2B_value.config(text='{0:.2f}'.format(tempMS2B) + u"\u2103")
                self.tempMS2B_value2.config(text=content[63:65])

                presMS1B = self.Base91_3(content[65:68])
                self.presMS1B_value.config(text='{0:.0f}'.format(presMS1B) + "Pa")
                self.presMS1B_value2.config(text=content[65:68])
                
                presMS2B = self.Base91_3(content[68:71])
                self.presMS2B_value.config(text='{0:.0f}'.format(presMS2B) + "Pa")
                self.presMS2B_value2.config(text=content[68:71])

                battVB = self.Base91_2(content[71:73])
                battVB = battVB / 4095.0 * 1.826 / 0.5
                self.battVB_value.config(text='{0:.3f}'.format(battVB) + "V")
                self.battVB_value2.config(text=content[71:73])

                lightB = self.Base91_2(content[73:75])
                lightB = 10.0**(math.log(1.002) / math.log(10) * lightB) / 139.0
                if lightB < 1.0:
                    self.lightB_value.config(text='{0:.4f}'.format(lightB) + "lux")
                elif lightB < 10.0:
                    self.lightB_value.config(text='{0:.3f}'.format(lightB) + "lux")
                elif lightB < 100.0:
                    self.lightB_value.config(text='{0:.2f}'.format(lightB) + "lux")
                else:
                    self.lightB_value.config(text='{0:.0f}'.format(lightB) + "lux")
                self.lightB_value2.config(text=content[73:75])

                self.out_label2_2.config(text="")
            else:
                self.latitudeB_value.config(text="")
                self.latitudeB_value2.config(text="")

                self.longitudeB_value.config(text="")
                self.longitudeB_value2.config(text="")

                self.altitudePB_value.config(text="")
                self.satsB_value.config(text="")
                self.resB_value.config(text="")
                self.data2_value2.config(text="")

                self.yearB_value.config(text="")
                self.monthB_value.config(text="")
                self.dayB_value.config(text="")
                self.hourB_value.config(text="")
                self.minuteB_value.config(text="")
                self.timeB_value.config(text="")
                self.data3_value2.config(text="")

                self.tempMCUB_value.config(text="")
                self.tempMCUB_value2.config(text="")

                self.tempTH1B_value.config(text="")
                self.tempTH1B_value2.config(text="")

                self.tempTH2B_value.config(text="")
                self.tempTH2B_value2.config(text="")

                self.tempMS1B_value.config(text="")
                self.tempMS1B_value2.config(text="")

                self.tempMS2B_value.config(text="")
                self.tempMS2B_value2.config(text="")

                self.presMS1B_value.config(text="")
                self.presMS1B_value2.config(text="")
                
                self.presMS2B_value.config(text="")
                self.presMS2B_value2.config(text="")

                self.battVB_value.config(text="")
                self.battVB_value2.config(text="")

                self.lightB_value.config(text="")
                self.lightB_value2.config(text="")
                
                self.out_label2_2.config(text="Packet without backlog.")
        else:
            self.out_label3.config(text="Not a valid packet!")
            self.out_label2_2.config(text="")

            self.latitude_value.config(text="")
            self.latitude_value2.config(text="")

            self.longitude_value.config(text="")
            self.longitude_value2.config(text="")

            self.altitude_value.config(text="")
            self.altitude_value2.config(text="")

            self.tempMCU_value.config(text="")
            self.tempMCU_value2.config(text="")

            self.tempTH1_value.config(text="")
            self.tempTH1_value2.config(text="")

            self.tempTH2_value.config(text="")
            self.tempTH2_value2.config(text="")

            self.tempMS1_value.config(text="")
            self.tempMS1_value2.config(text="")

            self.tempMS2_value.config(text="")
            self.tempMS2_value2.config(text="")

            self.presMS1_value.config(text="")
            self.presMS1_value2.config(text="")

            self.presMS2_value.config(text="")
            self.presMS2_value2.config(text="")

            self.battV_value.config(text="")
            self.battV_value2.config(text="")

            self.light_value.config(text="")
            self.light_value2.config(text="")

            self.altitudeP_value.config(text="")
            self.sats_value.config(text="")
            self.time_value.config(text="")
            self.res_value.config(text="")
            self.data1_value2.config(text="")

            self.latitudeB_value.config(text="")
            self.latitudeB_value2.config(text="")

            self.longitudeB_value.config(text="")
            self.longitudeB_value2.config(text="")

            self.altitudePB_value.config(text="")
            self.satsB_value.config(text="")
            self.resB_value.config(text="")
            self.data2_value2.config(text="")

            self.yearB_value.config(text="")
            self.monthB_value.config(text="")
            self.dayB_value.config(text="")
            self.hourB_value.config(text="")
            self.minuteB_value.config(text="")
            self.timeB_value.config(text="")
            self.data3_value2.config(text="")

            self.tempMCUB_value.config(text="")
            self.tempMCUB_value2.config(text="")

            self.tempTH1B_value.config(text="")
            self.tempTH1B_value2.config(text="")

            self.tempTH2B_value.config(text="")
            self.tempTH2B_value2.config(text="")

            self.tempMS1B_value.config(text="")
            self.tempMS1B_value2.config(text="")

            self.tempMS2B_value.config(text="")
            self.tempMS2B_value2.config(text="")

            self.presMS1B_value.config(text="")
            self.presMS1B_value2.config(text="")
                
            self.presMS2B_value.config(text="")
            self.presMS2B_value2.config(text="")

            self.battVB_value.config(text="")
            self.battVB_value2.config(text="")

            self.lightB_value.config(text="")
            self.lightB_value2.config(text="")

    def Base91_2(self, string):
        """ Decode a 2 symbol Base91 code """
        num = (ord(string[0]) - 33) * 91 + (ord(string[1]) - 33)
        return num

    def Base91_3(self, string):
        """ Decode a 3 symbol Base91 code """
        num = (ord(string[0]) - 33) * 8281 + (ord(string[1]) - 33) * 91 + (ord(string[2]) - 33)
        return num

    def Base91_4(self, string):
        """ Decode a 4 symbol Base91 code """
        num = (ord(string[0]) - 33) * 753571 + (ord(string[1]) - 33) * 8281 \
              + (ord(string[2]) - 33) * 91 + (ord(string[3]) - 33)
        return num

    def Base91_5(self, string):
        """ Decode a 5 symbol Base91 code """
        num = (ord(string[0]) - 33) * 68574961 + (ord(string[1]) - 33) * 753571 + (ord(string[2]) - 33) * 8281 \
              + (ord(string[3]) - 33) * 91 + (ord(string[4]) - 33)
        return num


#create the window
root = Tk()

#modify root window
root.title("TT7B APRS Packet Decoder")
root.geometry("495x820")

app = Application(root)

#kick off the event loop
root.mainloop()

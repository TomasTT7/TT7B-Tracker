# INPUT FORMAT:
# 2018-10-31 23:11:49 CET: OK7DMT-1>APRS,WIDE2-1,qAO,OK9STS:!/5LD\S*,yON2WW^%O,)Z!Lx,fQ-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
#
# OUTPUT FORMAT:
# 'yyyy-mm-dd HH:MM:SS timezone sender packet\n'
# 2018-10-31 23:11:49 CET OK7DMT-1 !/5LD\S*,yON2WW^%O,)Z!Lx,fQ-D33ZM0!<Xf5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z


from lxml import html
from HTMLParser import HTMLParser
import requests
import re


CALLSIGN = 'OK7DMT-1'               # '' returns packets from all callsigns
LIMIT = '25'                        # 5, 25, 50, 100, 300, 500, 1000


""" Source: https://stackoverflow.com/a/12982689 """
def cleanhtml(raw_html):
    cleanr = re.compile(r'<([^>]*)>')
    cleantext = re.sub(cleanr, '', raw_html)
    return cleantext


url = 'https://aprs.fi/?c=raw&call=' + CALLSIGN + '&limit=' + LIMIT + '&view=normal'

htmlparser = HTMLParser()

page = requests.get(url)
tree = html.fromstring(page.content)
packets = tree.xpath('//span[@class="raw_line"]')

existing = []
existing_raw = []

try:
    with open('APRSfi_grab ' + CALLSIGN + '.txt') as f:
        existing = f.readlines()
        f.close
except:
    pass
try:
    with open('APRSfi_grab ' + CALLSIGN + '_raw.txt') as f:
        existing_raw = f.readlines()
        f.close
except:
    pass

for packet in packets:
    temp = html.tostring(packet)
    temp = temp.replace('&gt;', '>').encode('utf-8')
    temp = temp.replace('&#194;', '').encode('utf-8')
    temp = temp.replace('&#160;', ' ').encode('utf-8')
    tempclean = cleanhtml(temp)

    temp1 = tempclean.split(' ', 3)
    temp2 = temp1[3].split('>', 1)
    temp3 = temp1[3].split(':', 1)
    
    out = temp1[0] + ' ' + temp1[1] + ' ' + temp1[2].replace(':', '').encode('utf-8') + ' ' + temp2[0] + ' ' + temp3[1]
    
    existing.append(out + '\n')
    existing_raw.append(tempclean + '\n')

existing_seen = set()
existing_raw_seen = set()

output = []
output_raw = []

for pkt in existing:
    if pkt not in existing_seen:
        output.append(pkt)
        existing_seen.add(pkt)

for pkt in existing_raw:
    if pkt not in existing_raw_seen:
        output_raw.append(pkt)
        existing_raw_seen.add(pkt)

output.sort()
output_raw.sort()

f = open('APRSfi_grab ' + CALLSIGN + '.txt', 'w')
fr = open('APRSfi_grab ' + CALLSIGN + '_raw.txt', 'w')

for line in output:
    f.write(line.encode('utf8'))
    print(line[:-1])

for line in output_raw:
    fr.write(line.encode('utf8'))

f.close
fr.close









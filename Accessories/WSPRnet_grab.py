from lxml import html
import requests
import re


CALLSIGN = 'OK7DMT'
LIMIT = '10'
BAND = '20'                 # all, u, 432, 220, 2, 4, 6, 10, 12, 15, 17, 20, 30 40, 60, 80, 160, 2190


# Grab latest messages
url = 'http://wsprnet.org/olddb?mode=html&band=' + BAND + '&limit=' + LIMIT + '&findcall=' + CALLSIGN + '&findreporter=&sort=date'

page = requests.get(url)
tree = html.fromstring(page.content)
packets = tree.xpath('//tr[@id="oddrow"] | //tr[@id="evenrow"]')
packets.pop(0)


# Load existing data
msgs = []

try:
    with open('WSPRnet_grab ' + CALLSIGN + '.txt') as f:
        msgs = f.readlines()
        f.close
except:
    pass


# Parse data
data = []

for packet in packets:
    data.append([c.text for c in packet.getchildren()])

for i in range(len(data)):
    _msg = ''
    first = True
    
    for y in range(len(data[i])):
        if first:
            first = False
            _msg = re.sub('[\xa0]', '', data[i][y])
        else:
            _msg = _msg + ',' + re.sub('[\xa0]', '', data[i][y])
    
    msgs.append(_msg + '\n')


# Store new messages in a file
msgs_seen = set()
output = []

for i in msgs:
    if i not in msgs_seen:
        output.append(i)
        msgs_seen.add(i)

output.sort()

f = open('WSPRnet_grab ' + CALLSIGN + '.txt', 'w')

for line in output:
    f.write(line.encode('utf8'))
    print(line[:-1])

f.close



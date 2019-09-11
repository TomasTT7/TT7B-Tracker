from lxml import html
from HTMLParser import HTMLParser
import requests
import re


CALLSIGN = 'OK7DMT-1'               # '' returns packets from all callsigns
LIMIT = '25'                        # 5, 25, 50, 100, 300, 500, 1000
EMAIL = ''  						# aprs.fi requires login to show raw packets
PASSWORD = ''


""" Source: https://stackoverflow.com/a/12982689 """
def cleanhtml(raw_html):
    cleanr = re.compile(r'<([^>]*)>')
    cleantext = re.sub(cleanr, '', raw_html)
    return cleantext


url_login = 'https://aprs.fi/?c=login'
url_data = 'https://aprs.fi/?c=raw&call=' + CALLSIGN + '&limit=' + LIMIT + '&view=normal'


htmlparser = HTMLParser()

session_requests = requests.session()
page = session_requests.get(url_login)

tree = html.fromstring(page.text)
csrf = list(set(tree.xpath("//input[@name='csrf_login']/@value")))[0]

values = {'email': EMAIL,
          'password': PASSWORD,
          'csrf_login': csrf,
          'do_login': 1}

r = session_requests.post(url_login, data=values, headers = dict(referer=url_login))

page_data = session_requests.get(url_data, headers = dict(referer = url_data))
tree_data = html.fromstring(page_data.content)
packets = tree_data.xpath('//span[@class="raw_line"] | //span[@class="raw_line_err"]')

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
    
    tempclean = cleanhtml(temp)
    tempclean = tempclean.replace('&gt;', '>').encode('utf-8')
    tempclean = tempclean.replace('&lt;', '<').encode('utf-8')
    tempclean = tempclean.replace('&amp;', '&').encode('utf-8')
    tempclean = tempclean.replace('&quot;', '"').encode('utf-8')
    tempclean = tempclean.replace('&apos;', '\'').encode('utf-8')
    tempclean = tempclean.replace('&#34;', '"').encode('utf-8')
    tempclean = tempclean.replace('&#38;', '&').encode('utf-8')
    tempclean = tempclean.replace('&#39;', '\'').encode('utf-8')
    tempclean = tempclean.replace('&#60;', '<').encode('utf-8')
    tempclean = tempclean.replace('&#62;', '>').encode('utf-8')
    tempclean = tempclean.replace('&#160;', ' ').encode('utf-8')
    tempclean = tempclean.replace('&#194;', '').encode('utf-8')

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

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
    cleanr = re.compile('<.*?>')
    cleantext = re.sub(cleanr, '', raw_html)
    return cleantext


url = 'https://aprs.fi/?c=raw&call=' + CALLSIGN + '&limit=' + LIMIT + '&view=normal'

htmlparser = HTMLParser()

page = requests.get(url)
tree = html.fromstring(page.content)
packets = tree.xpath('//span[@class="raw_line"]')

f = open('APRSfi_grab ' + CALLSIGN + '.txt', 'a')
fr = open('APRSfi_grab ' + CALLSIGN + '_raw.txt', 'a')

for packet in packets:
    processed = htmlparser.unescape(cleanhtml(html.tostring(packet)).replace('&#194;', ''))
    temp1 = processed.split(' ', 3)
    temp2 = temp1[3].split('>', 1)
    temp3 = temp1[3].split(':', 1)
    
    out = temp1[0] + ' ' + temp1[1] + ' ' + temp1[2].replace(':', '') + ' ' + temp2[0] + ' ' + temp3[1]
    print(out)
    f.write(out.encode('utf8') + '\n')
    fr.write(processed.encode('utf8') + '\n')

f.close()
fr.close()

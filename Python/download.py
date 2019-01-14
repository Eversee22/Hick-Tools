from urllib import request
from urllib.error import HTTPError, URLError
import os
import time
import socket

path = 'IMAGES/'
if not os.path.exists(path):
	os.mkdirs(path)
IGNORE_EXISTF = True
if IGNORE_EXISTF:
	existf = [f for f in os.listdir(path)]

urlf = 'urls_.txt'
timeout = 10

#for urllib.request.urlretrieve
def reporthook(bnum,size,totsize):
	if len(filesize)==0:
		filesize.append(totsize)
	perc = 100 * bnum * size / totsize
	print('%.2f%%' % perc )

count = 0
with open(urlf,'r') as f:
	start_dw = time.time()
	for line in f:
		url = line.rstrip()
		fname = url.split('/')[-1]
		if IGNORE_EXISTF and fname in existf:
			print('%s already exists, skip\n' % (fname))
			continue
		count += 1
		print('[{1}] download file:{0}'.format(fname,count))
		start = time.time()
		try:
			r = request.urlopen(url, timeout=timeout)
			#print('Access successful.')
			r.raise_for_status()
			of = open(path+fname,'wb')
			of.write(r.read())
			of.close()
			r.close()
			print('#over, successful#')
		except Exception as error:
				print('##error:%s - URL:%s##'%(error,url))
		print()
		#print('over, spend time {:.3f}'.format(time.time()-start)) 
	end_dw = time.time()	
	f.close()
print('over, spend total time:{:.3f}'.format(end_dw-start_dw))

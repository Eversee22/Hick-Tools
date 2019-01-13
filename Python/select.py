import sys

n = 500
types = ['neutral','drawings','sexy']
path = './raw_data/'
if len(sys.argv)>=2:
	ftype = types[int(sys.argv[1])]
else:
	print('usage: type(0:neutral,1:drawings,2:sexy), [number(default 500)]')
	exit(0)
if len(sys.argv)>=3:
	n = int(sys.argv[2])
path += ftype+'/'
fp = open(path+'urls_'+ftype+'_org'+'.txt','r')
pic_urls = []
for i in range(n):
	try:
		pic_urls.append(fp.readline())
	except Exception as e:
		print(e)
		break
fp.close()
print('urls number:{}'.format(len(pic_urls)))
ofp = open(path+'urls_'+ftype+'.txt','w')
for url in pic_urls:
	ofp.write(url)
ofp.close()
print('over')


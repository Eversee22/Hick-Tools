from http.server import HTTPServer, BaseHTTPRequestHandler
from io import BytesIO, StringIO
import os
import sys
import socket
import re
import argparse


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

	def do_GET(self):
		f = StringIO()
		f.write('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">')
		f.write("<html>\n<title>Simtex</title>\n")
		f.write("<body>\n<h2>Simtex</h2>\n")
		f.write("<hr>\n")
		f.write("<form ENCTYPE=\"multipart/form-data\" method=\"post\">")
		f.write("<textarea name=\"simtex\"></textarea>")
		f.write("<input type=\"submit\" value=\"upload\"/>")
		f.write("              ")
		f.write("<input type=\"button\" value=\"HomePage\" onClick=\"location='/'\">")
		f.write("</form>\n")
		f.write("<hr>\n</body>\n</html>\n")
		self.send_response(200)
		self.send_header("Content-type", "text/html")
		self.send_header("Content-Length", str(f.tell()))
		self.end_headers()
		self.wfile.write(f.getvalue().encode('gbk'))
		f.close()

	def do_POST(self):
		response = BytesIO()
		content_length = int(self.headers['Content-Length'])
		boundary = self.headers['Content-Type'].split('=')[1].encode('utf-8')
		# print(boundary)
		line = self.rfile.readline()
		content_length -= len(line)
		if not boundary in line:
			print("Content not begin with boundary.")
			return
		line = self.rfile.readline()
		content_length -= len(line)
		name = re.findall(r'Content-Disposition.*name="(.*)"', line.decode('utf-8'))
		# print(name)
		if name[0] != 'simtex':
			self.send_error(404, 'Unknown value')
			return
		line = self.rfile.readline()
		content_length -= len(line)
		
		path = os.getcwd()
		words = filter(None, self.path.split('/'))
		for word in words:
			path = os.path.join(path, word)
		with open(os.path.join(path, 'simtex'), 'ab') as f:
			f.write(b'-'*10)
			f.write(b'\n')
			preline = self.rfile.readline()
			content_length -= len(preline)
			read_s = 0
			read_c = b''
			while content_length > 0:
				line = self.rfile.readline()
				content_length -= len(line)
				if boundary in line:
					preline = preline[:-1]
					# if preline.endswith(b'\r'):
						# preline = preline[:-1]
					f.write(preline)
					read_c += preline
					read_s += len(preline)
					# print(preline)
					# print('read size:', read_s)
					break
				else:
					f.write(preline)
					read_c += preline
					read_s += len(preline)
					preline = line
			f.write(b'\n\n')
			
		self.send_response(200)
		self.end_headers()
		# body = self.rfile.read(content_length)
		# response.write(b'This is POST request. ')
		response.write(('Received, %d bytes.\n' % read_s).encode('utf-8'))
		response.write(read_c)
		self.wfile.write(response.getvalue())

def get_ip_address():
	try:
		if sys.platform == 'win32':
			myip = socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET, socket.SOCK_DGRAM)[-1][4][0]
		else:
			import fcntl
			s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
			myip = socket.inet_ntoa(fcntl.ioctl(
				s.fileno(),
				0x8915,  # SIOCGIFADDR
				struct.pack('256s', ifname[:15])
			)[20:24])
	except Exception as e:
		print(e)
		myip = "127.0.0.1"
	
	return myip
	
def parse_arg():
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument('-p', dest='port', help='port', default=8322, type=int)
	arg_parser.add_argument('--root-dir', dest='root_dir', help='root dir', default='./')
	
	return arg_parser.parse_args()
	
args = parse_arg()
root_dir = args.root_dir
port = args.port
myip = get_ip_address()

os.chdir(root_dir)
httpd = HTTPServer((myip, port), SimpleHTTPRequestHandler)
print('Listening ...', 'http://{}:{}'.format(myip, port))
httpd.serve_forever()

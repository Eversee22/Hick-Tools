import os, sys, posixpath, html, shutil
from http.server import HTTPServer, BaseHTTPRequestHandler
from socketserver import ThreadingMixIn
from urllib import parse
from io import BytesIO, StringIO, BufferedReader
import time
import mimetypes
import re
import platform
import socket
import argparse

# import importlib

__version__ = '1.0'
osType = platform.system()
charencoding = 'utf8'
block_s = 2 ** 29  # 512 MB


def parse_arg():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-p', dest='port', help='port', default=8321, type=int)
    arg_parser.add_argument('--root-dir', dest='root_dir', help='root dir', default='./')
    arg_parser.add_argument('-if', dest='ifname', help='net interface name', default=None)

    return arg_parser.parse_args()


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    server_info = "Simple HTTP File Transfer/" + __version__

    if not mimetypes.inited:
        mimetypes.init()
    extensions_map = mimetypes.types_map.copy()
    extensions_map.update({
        '': 'application/octet-stream',
        '.py': 'text/plain',
        '.c': 'text/plain',
        '.h': 'text/plain',
    })

    print(server_info)

    def do_GET(self):
        f = self.send_head()
        if f:
            if isinstance(f, StringIO):
                bytes = f.getvalue().encode(charencoding)
                self.wfile.write(bytes)
            elif isinstance(f, BufferedReader):
                f.seek(0, 2)
                file_s = f.tell()
                f.seek(0)
                read_s = 0
                bytes = f.read(block_s)
                read_s += len(bytes)
                while len(bytes) > 0:
                    self.wfile.write(bytes)
                    print('down {:.1f}%'.format(read_s / file_s * 100))
                    bytes = f.read(block_s)
                    read_s += len(bytes)
            f.close()

    def do_HEAD(self):
        """Serve a HEAD request."""
        f = self.send_head()
        if f:
            f.close()

    def do_POST(self):
        r, info = self.deal_post_data()
        print(r, info, "by: ", self.client_address)
        f = StringIO()
        f.write('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">')
        f.write("<html>\n<title>Upload Result Page</title>\n")
        f.write("<body>\n<h2>Upload Result Page</h2>\n")
        f.write("<hr>\n")
        if r:
            f.write("<strong>Success:</strong>")
        else:
            f.write("<strong>Failed:</strong>")
        f.write(info)
        f.write("<br><a href=\"%s\">back</a>" % self.headers['referer'])
        # f.write("<hr><small>Powered By: bones7456, check new version at ")
        # f.write("<a href=\"http://li2z.cn/?s=SimpleHTTPServerWithUpload\">")
        f.write("</body>\n</html>\n")
        length = f.tell()
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", str(length))
        self.end_headers()
        if f:
            self.wfile.write(f.getvalue().encode('utf-8'))
            f.close()

    def deal_post_data(self):
        # print("headers:\n", self.headers)
        boundary = self.headers['Content-Type'].split('=')[1]
        boundary = boundary.encode('utf-8')
        remainbytes = int(self.headers['Content-Length'])
        line = self.rfile.readline()
        remainbytes -= len(line)
        # print(line)
        if not boundary in line:
            return (False, "Content not begin with boundary.")
        line = self.rfile.readline()
        remainbytes -= len(line)
        # print(line)
        try:
            strline = line.decode('utf-8')
        except UnicodeDecodeError:
            try:
                strline = line.decode('gbk')
            except Exception:
                strline = str(line)        
        fn = re.findall(r'Content-Disposition.*name="file"; filename="(.*)"', strline)
        # print('filename:',fn)
        if not fn or len(fn[0]) == 0:
            # print(self.rfile.read())
            return (False, "Can't find out file name ...")
        path = self.translate_path(self.path)
        try:
            if osType == "Linux":
                fn = os.path.join(path, fn[0])
            else:
                fn = os.path.join(path, fn[0])
        except Exception as e:
            return (False, str(e))

        try:
            base, suf = fn.split('.')
        except ValueError:
            base = fn
            suf = ''
        while os.path.exists(fn):
            base += '_'
            fn = base + '.' + suf

        for i in range(2):
            line = self.rfile.readline()
            remainbytes -= len(line)
        # print(line)

        try:
            out = open(fn, 'wb')
        except IOError:
            return (False, "Can't create file to write, do you have permission to write?")

        preline = self.rfile.readline()
        # file_s = remainbytes - len(boundary)
        remainbytes -= len(preline)
        # print(preline)
        # print('file size:',file_s)
        read_s = 0
        # block_c = 1
        while remainbytes > 0:
            line = self.rfile.readline()
            remainbytes -= len(line)
            if boundary in line:
                # print(line)
                preline = preline[:-1]
                if preline.endswith(b'\r'):
                    preline = preline[:-1]
                out.write(preline)
                read_s += len(preline)
                # print('up {:.2f}%'.format(read_s/file_s*100))
                out.close()
                # print('read size:', read_s)
                print('remainbytes:', remainbytes)
                return (True, "File '%s' upload success, size: %d bytes" % (fn, read_s))
            else:
                out.write(preline)
                read_s += len(preline)
                # if int(read_s / file_s * 100) > block_c * 20:
                # print('up {:.2f}%'.format(read_s/file_s*100))
                # block_c += 1
                preline = line

        return (False, "Unexpect Ends of data.")

    def translate_path(self, path):
        # path = path.split('?',1)[0].split('#',1)[0]
        path = posixpath.normpath(parse.unquote(path))
        words = filter(None, path.split('/'))
        path = os.getcwd()
        for word in words:
            # print(word)
            drive, word = os.path.splitdrive(word)
            # print(drive, word)
            head, word = os.path.split(word)
            # print(head, word)
            if word in (os.curdir, os.pardir):
                continue
            path = os.path.join(path, word)
        return path

    def list_directory(self, path):
        try:
            file_list = os.listdir(path)
        except os.error:
            self.send_error(404, "No permission to list directory")
            return None

        file_list.sort(key=lambda a: a.lower())
        f = StringIO()
        displaypath = html.escape(parse.unquote(self.path))
        f.write('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">')
        f.write("<html>\n<title>Directory listing for %s</title>\n" % displaypath)
        f.write("<body>\n<h2>Directory listing for %s</h2>\n" % displaypath)
        f.write("<hr>\n")
        f.write("<form ENCTYPE=\"multipart/form-data\" method=\"post\">")
        f.write("<input name=\"file\" type=\"file\"/>")
        # f.write("<input name=\"simtex\" type=\"text\"/>")
        f.write("<input type=\"submit\" value=\"upload\"/>")
        f.write("              ")
        f.write("<input type=\"button\" value=\"HomePage\" onClick=\"location='/'\">")
        f.write("</form>\n")
        f.write("<hr>\n<ul>\n")
        # print(self.path)
        if len(self.path) > 1:
            file_list.insert(0, os.pardir)
        for name in file_list:
            fullname = os.path.join(path, name)
            colorName = displayname = linkname = name
            if os.path.isdir(fullname):
                colorName = '<span style="background-color: #CEFFCE;">' + name + '/</span>'
                displayname = name
                linkname = name + "/"

            if os.path.islink(fullname):
                colorName = '<span style="background-color: #FFBFFF;">' + name + '@</span>'
                displayname = name

            filename = os.getcwd() + '/' + displaypath + displayname
            f.write(
                '<table><tr><td width="60%%"><a href="%s">%s</a></td><td width="20%%">%s</td><td width="20%%">%s</td></tr>\n'
                % (parse.quote(linkname), colorName,
                   sizeof_fmt(os.path.getsize(filename)), modification_date(filename)))
        f.write("</table>\n<hr>\n</body>\n</html>\n")  # html over
        length = f.tell()
        # f.seek(0)
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", str(length))
        self.end_headers()

        return f

    def guess_type(self, path):
        base, ext = posixpath.splitext(path)
        if ext in self.extensions_map or ext.lower() in self.extensions_map:
            return self.extensions_map[ext]
        else:
            return self.extensions_map['']

    def send_head(self):
        path = self.translate_path(self.path)
        f = None
        if os.path.isdir(path):
            if not self.path.endswith('/'):
                # redirect browser - doing basically what apache does
                self.send_response(301)
                self.send_header("Location", self.path + "/")
                self.end_headers()
                return None
            hp = False
            for i in "index.html", "index.htm":
                i = os.path.join(path, i)
                if os.path.exists(i):
                    path = i
                    hp = True
                    break
            if not hp:
                # stringio = self.list_directory(path)
                # bytes = stringio.getvalue().encode(charencoding)
                # stringio.close()
                return self.list_directory(path)

        ctype = self.guess_type(path)
        try:
            f = open(path, 'rb')
        except IOError:
            self.send_error(404, "File not found")
            return None
        self.send_response(200)
        self.send_header("Content-Type", ctype)
        fs = os.fstat(f.fileno())
        self.send_header("Content-Length", str(fs[6]))
        self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
        self.end_headers()

        # bytes = f.read()
        # f.close()

        return f


class ThreadingServer(ThreadingMixIn, HTTPServer):
    pass


def sizeof_fmt(num):
    for x in ['bytes', 'KB', 'MB', 'GB']:
        if num < 1024.0:
            return "%3.1f%s" % (num, x)
        num /= 1024.0
    return "%3.1f%s" % (num, 'TB')


def modification_date(filename):
    return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(os.path.getmtime(filename)))


def get_ip_address(ifname=None):
    try:
        if sys.platform == 'win32':
            addrs = socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET, socket.SOCK_DGRAM)
            for addr in addrs:
                print(addr[4][0])
            si = input('which to use: ')
            if len(si) == 0:
                i = -1
            else:
                i = int(si)
            myip = addrs[i][4][0]
        else:
            import fcntl, struct
            if ifname is None:
                return "0.0.0.0"
            if isinstance(ifname, str):
                ifname = bytes(ifname, 'utf-8')
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            myip = socket.inet_ntoa(fcntl.ioctl(
                s.fileno(),
                0x8915,  # SIOCGIFADDR
                struct.pack('256s', ifname[:15])
            )[20:24])
    except Exception as e:
        print(e)
        myip = "0.0.0.0"

    return myip


args = parse_arg()
root_dir = args.root_dir
port = args.port
myip = get_ip_address(args.ifname)
serve_addr = (myip, port)

os.chdir(root_dir)
httpd = ThreadingServer(serve_addr, SimpleHTTPRequestHandler)
print('Listening ...', 'http://{}:{}'.format(myip, port))
httpd.serve_forever()

#!/usr/bin/env python3

import os, cgitb, sys
cgitb.enable()

def is_multipart(content_type):
	if content_type == None or (len(content_type) > len("multipart/form-data; boundary=")\
	and content_type[:30] == "multipart/form-data; boundary="):
		return True
	return False

upload_pass = os.getenv("UPLOAD_PASS")
body = sys.stdin.read()
content_type = os.getenv("CONTENT_TYPE")

# eror checking
if not is_multipart(content_type):
	exit(415)
if upload_pass == None or upload_pass == "":
	exit(403)

boundary = content_type[content_type.find("boundary=") + 9:]

# finding file body
begin = body.find('\r\n\r\n') + 4
end = body.find('\r\n--' + boundary + '--')
file_body = body[begin:end]

# finding filename and path
b1 = body.find('--' + boundary)
b1end = b1 + len(boundary) + 2
fnamebegin = b1end + body[b1end:].find('filename="') + 10
fnameend = fnamebegin + body[fnamebegin:].find('"\r\n')
file_name = body[fnamebegin:fnameend]

if upload_pass[-1] != '/':
	upload_pass += '/'

file_path = upload_pass + file_name

try:
	fp = open(file_path, 'w')
except:
	exit(403)

fp.write(file_body)

fp.close()

response_body = f'''<HTML><HEAD><meta http-equiv="content-type" content="text/html;charset=utf-8">
<TITLE>Upload successful</TITLE></HEAD><BODY>
<H1>{file_name} has been uploaded</H1>
</BODY></HTML>'''

print(f'''HTTP/1.1 200 OK\r
Content-Type: text/html; charset=UTF-8\r
Content-Length: {len(response_body)}\r
\r
{response_body}''')

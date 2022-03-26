#!/usr/bin/env python3

import os, cgitb, sys
#cgitb.enable()

upload_pass = os.getenv("UPLOAD_PASS")
body = sys.stdin.read()
content_type = os.getenv("CONTENT_TYPE")
boundary = content_type[content_type.find("boundary=") + 9:]

#finding file body
begin = body.find('\r\n\r\n') + 4
end = body.find('\r\n--' + boundary + '--')
file_body = body[begin:end]

#finding filename and path
b1 = body.find('--' + boundary)
b1end = b1 + len(boundary) + 2
fnamebegin = b1end + body[b1end:].find('filename="') + 10
fnameend = fnamebegin + body[fnamebegin:].find('"\r\n')
file_name = body[fnamebegin:fnameend]

if upload_pass[-1] != '/':
	upload_pass += '/'

file_path = upload_pass + file_name

fp = open(file_path, 'w')

fp.write(file_body)

fp.close()

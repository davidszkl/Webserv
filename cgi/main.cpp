#include "cgi.hpp"

#include <iostream>
#include <string>





int main()
{
	string full_message, root, location, upload_pass;
	full_message = "\
POST /upload.py HTTP/1.1\r\n\
Host: 4242\r\n\
Accept:  text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n\
Accept-Encoding:  gzip, deflate\r\n\
Accept-Language:  en-US,en;q=0.5\r\n\
Connection:  keep-alive\r\n\
Content-Length:  253\r\n\
Content-Type:  multipart/form-data; boundary=---------------------------186655929040397696981263301363\r\n\
Dnt:  1\r\n\
Origin: 4242\r\n\
Referer: 4242/upload\r\n\
Sec-Fetch-Dest:  document\r\n\
Sec-Fetch-Mode:  navigate\r\n\
Sec-Fetch-Site:  same-origin\r\n\
Sec-Fetch-User:  ?1\r\n\
Sec-Gpc:  1\r\n\
Upgrade-Insecure-Requests:  1\r\n\
User-Agent: 98.0) Gecko/20100101 Firefox/98.0\r\n\
\r\n\
-----------------------------186655929040397696981263301363\r\n\
Content-Disposition: form-data; name=\"file1\"; filename=\"ok.txt\"\r\n\
Content-Type: text/plain\r\n\
\r\n\
je suis beau\n\
et \n\
\n\
\n\
magnig=fique\n\
;\n\
\r\n\
-----------------------------186655929040397696981263301363--\r\n";
	root = "/media/martin/Bowser/webserv/cgi/cgi-bin";
	location = "/";
	upload_pass = "/tmp";
	upload_pass = "";
	if (is_valid_for_cgi(full_message, root, location, 50))
		execute_cgi(full_message, root, location, upload_pass, 1);
}




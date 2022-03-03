#include "cgi.hpp"

#include <string>

int main()
{
	std::string full_message = "\
GET /script.py/with/more HTTP/1.1\r\n\
Host: localhost:4269\r\n\
Connection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.102 Safari/537.36\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n\
Sec-GPC: 1\r\n\
Sec-Fetch-Site: none\r\n\
Sec-Fetch-Mode: navigate\r\n\
Sec-Fetch-User: ?1\r\n\
Sec-Fetch-Dest: document\r\n\
Accept-Encoding: gzip, deflate, br\r\n\
Accept-Language: en-US,en;q=0.9,fr;q=0.8\r\n\
Cookie: wp-settings-time-1=1645023355\r\n\
\r\n\
		"; // no body here (because it's a get)
		
		if (is_valid_for_cgi(full_message, "./"))
			execute_cgi(full_message, 1);
}

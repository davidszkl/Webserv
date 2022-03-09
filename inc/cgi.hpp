#pragma once
#include <string>

/*
	checks if http message (header + body) is valid for a cgi response.
	full_message is the whole http message received from the client.
	root is the root of the server.
	Return 1 on success, 0 on failure and 415 if valid but unsupported Content-Type;
	If return 415, you should send 415 error page
 */
int is_valid_for_cgi(const std::string& full_message, std::string root, const std::string& location);

/*
	This function needs the first line of the http header and the body, so pass the full http message in full_message.
	root is the root of the server.
	ouput_fd is where the python script will write its ouput.
	The http message should be valid! (is_valid_for_cgi must have returned true with the same message/root)
 */
void execute_cgi(const std::string& full_message, std::string root, const std::string& location, int ouput_fd);

/*
header must end with \r\n\r\n. Everything after that is ignored.
if there is Content-Length: 200 in the header and name=="Content-Length"m then returns "200"
returns "" if not found
 */
std::string get_header_info(const std::string& header, const std::string& name);


//example test:
/*
#include "cgi.hpp"

#include <iostream>
#include <string>

int main()
{
	std::string full_message, root, location;
	full_message = "\
POST /you/must/exec/cgi-script.py/ok/cool?cool=skdjklfj%%%% HTTP/1.1\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: 5\r\n\
\r\n\
0123456789EOF";
	root = "/media/martin/Bowser/webserv/cgi/cgi-bin";
	location = "/you/must/exec";
	if (is_valid_for_cgi(full_message, root, location))
		execute_cgi(full_message, root, location, 1);
}


*/

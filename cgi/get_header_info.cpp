#include <string>
#include "debug.hpp"

/*
header must end with \r\n\r\n. Everything after that is ignored.
if there is Content-Length: 200 in the header and name=="Content-Length"m then returns "200"
returns "" if not found
 */

std::string get_header_info(const std::string& header, const std::string& name)
{
	const std::size_t end = header.find("\r\n\r\n");
	const std::string to_find = "\r\n" + name + ": ";
	const std::size_t n = header.find(to_find);
	if (n == std::string::npos || n >= end)
		return "";
	const std::size_t l = std::string(&header[n + to_find.length()]).find("\r\n");
	return header.substr(n + to_find.length(), l);
}

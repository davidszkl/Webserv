#include "cgi.hpp"
#include "debug.hpp"

#include <exception>
#include <unistd.h>
#include <stdlib.h>

/*
	e.g.: if s == "hello ok" return "hello"
	s must not begin with a space character
 */
static std::string get_next_word(const std::string& s)
{
	std::string rval;
	for(std::size_t i = 0; i < s.length() && s[i] != ' '; i++)
		rval += s[i];
	return rval;
}


static std::size_t get_next_slash(const std::string& s)
{
	std::size_t i;
	for (i = 0; i < s.length(); i++)
		if (s[i] == '/')
			return i;
	return i;
}

static bool is_py(const std::string file)
{
	std::size_t l = file.length();
	return file[l - 1] == 'y' && file[l - 2] == 'p' && file[l - 3] == '.';
}

static bool is_exec(const std::string& file_path, const std::string& root)
{

	const bool b = (access((root + file_path).c_str(), X_OK) != -1);
	if (b)
		logn(root + file_path + " is an executable file");
	else
		logn(root + file_path + " is not an executable file");
	return b;
}

/*
	checks if there is a valid executable .py file in the header path.
	return std::string::npos if nothnig found, otherwise index of character after the 'y' in the .py
 */
static std::size_t get_end_path(const std::string& path, const std::string& root)
{
	for (std::size_t i = 0; i < path.length(); i++)
	{
		if (path[i] == '/' && i + 1 != path.length())
		{
			std::string file = path.substr(i+1, get_next_slash(&path[i+1]));
			if (is_py(file) && is_exec(path.substr(0, i + 1 + file.length()), root))
				return i + 1 + file.length();
		}
	}
	return std::string::npos;
}

/*
	checks if http message (header + body) is valid for a cgi response.
	full_message is the whole http message received from the client.
	root is the root of the server.
	Return 1 on success, 0 on failure and 415 if valid but unsupported Content-Type;
	If return 415, you should send 415 error page
 */
int is_valid_for_cgi(const std::string& full_message, std::string root)
{
	if (root[root.length() -1] != '/') root += '/';
	logn("Checking if message is valid for cgi...");
	using std::string;
	const string request = get_next_word(full_message);
	if (0 != access("/usr/bin/env", X_OK))
	{
		logn("/usr/bin/env is not executable. Python CGI needs it in sha-bang. is_valid_for _cgi returned false");
		return false;
	}
	if (request != "GET" && request != "POST")
	{
		logn("Request ivalid for cgi: request==" + request);
		return false;
	}
	const string path = get_next_word(&full_message[request.length() + 1]);
	const std::size_t path_end = get_end_path(path, root);
	if (path_end == string::npos)
	{
		logn("Path is invalid for cgi: path==" + path + " --- root==" + root);
		return false;
	}
	if (request == "POST")
	{
		std::string content_type = get_header_info(full_message, "Content-Type");
		if (content_type != "application/x-www-form-urlencoded")
		{
			logn("CGI does not support this Content-Type: " + content_type);
			logn("is_valid_for_cgi returned 415");
			return 415;
		}
		std::string content_length = get_header_info(full_message, "Content-Length");
		if (content_length == "")
		{
			logn("No Content-Length in header. is_valid_for cgi is returning false");
			return false;
		}
		std::size_t cl;
		try{
			cl = std::atoi(content_length.c_str());
		} catch(...)
		{
			logn(content_length + " is an invalid Content-Legnth. is_valid_for cgi returns false");
			return false;
		}
		if (full_message.find("\r\n\r\n") + cl + 4 >= full_message.length())
		{
			logn("Content-Length too large is_valid_for_cgi returns false");
			return false;
		}
	}
	logn("http message is valid for cgi");
	return true;
}

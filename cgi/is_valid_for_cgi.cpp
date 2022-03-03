#include "cgi.hpp"
#include "debug.hpp"

#include <exception>
#include <unistd.h>

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
	return access((root + file_path).c_str(), X_OK) != -1;
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
	full_message can be the whole http message received from the client,
	but only the first line will be parsed (the line that contains REQUEST PATH PROTOCOL\r\n)
	root is the root of the server.
 */
bool is_valid_for_cgi(const std::string& full_message, const std::string& root)
{
	logn("Checking if message is valid for cgi...");
	using std::string;
	const string request = get_next_word(full_message);
	if (request != "GET" && request != "POST")
	{
		logn("Request ivalid for cgi: neither GET nor POST");
		return false;
	}
	const string path = get_next_word(&full_message[request.length() + 1]);
	const std::size_t path_end = get_end_path(path, root);
	if (path_end == string::npos)
	{
		logn("Path is invalid for cgi: path==" + path + " --- root==" + root);
		return false;
	}
	logn("http message is valid for cgi");
	return true;
}

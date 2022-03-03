#include "cgi.hpp"
#include "debug.hpp"
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
	This function needs the first line of the http header and the body, so pass the full http message in full_message.
	root is the root of the server.
	ouput_fd is where the python script will write its ouput.
	The http message should be valid! (is_valid_for_cgi must have returned true with the same message/root)
 */
void execute_cgi(const std::string& full_message, const std::string& root, int ouput_fd)
{
	using std::string;
	logn("Executing cgi...");
	const string request = get_next_word(full_message);
	const string path = get_next_word(&full_message[request.length() + 1]);
	const std::size_t path_end = get_end_path(path, root);
	const std::string exec_path = root + path.substr(0, path_end);
	logn("path to cgi executable==" + exec_path);
	logn("cgi request==" + request);
	std::size_t qpos = path_end;
	while (qpos < path.length() && path[qpos] != '?')
		qpos++;
	const string path_info = path.substr(path_end, qpos - path_end);
	const bool define_path = path_info != "";
	logn("path_info==" + path_info);
	string query_string;
	bool define_query = false;
	if (request == "GET" && qpos != path.length())
	{
		query_string = path.substr(qpos + 1, string::npos);
		define_query = true;
	}
	else if (request == "POST" && has_body(full_message)) //shoudl check if Content-Length is not zero(if not multipart) and if there is something after the \r\n\r\n
	{
		query_string = get_query(full_message); //should unchunck it if multipart
		define_query = true;
	}
//if post && define_query ->  pass it to stdin
//if get && define query -> pass it as env
//if define path -> pass it as env
//gl
}



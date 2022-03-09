#include "cgi.hpp"
#include "debug.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>

char	**ft_split(char const *s, char c);

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

static void ft_freetab(const char **tab)
{
	for (std::size_t i = 0; tab[i]; i++)
		free((void *)tab[i]);
	free(tab);
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

//if post && define_query ->  pass it to stdin
//if get && define query -> pass it as env
//if define path -> pass it as env
static void setup_and_exec(int output_fd,
		std::string request,
		bool define_query,
		std::string query_string,
		bool define_path,
		std::string path_info,
		std::string exec_path)
{
	std::string command = "/usr/bin/env";
	std::string exec_path2 = exec_path;
	while (exec_path2[exec_path2.length() - 1] != '/')
		exec_path2 = exec_path2.substr(0, exec_path2.length() - 1);
	command += " --chdir=" + exec_path2;
	if (define_query && request == "GET")
		command += " QUERY_STRING=" + query_string;
	if (define_path)
		command += " PATH_INFO=" + path_info;
	command += " REQUEST_METHOD=" + request;
	command += " " + exec_path;
	int pipefds[2];
	if (pipe(pipefds) == -1)
	{
		perror("pipe()");
		return;
	}
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork()");
			return;
	} else if (pid == 0) {	
		close(pipefds[1]);
		if (-1 == dup2(pipefds[0], 0)) {
			close(pipefds[0]);
			close(output_fd);
			perror("dup2()"); exit(1);
		}
		close(pipefds[0]);
		if (-1 == dup2(output_fd, 1))   {
			if (output_fd != 1) close(output_fd);
			perror("dup2()"); exit(1);
		}
		if (output_fd != 1) close(output_fd);
		const char **split = const_cast<const char**>(ft_split(command.c_str(), ' '));
		if (!split) { perror("ft_split()"); exit(1); }
		logn("executing " + exec_path + ":");
		execve(split[0], (char * const *)split, 0);
		ft_freetab(split);
		perror("execve()");
		exit(1);
	}
	if (request == "POST" && define_query)
		write(pipefds[1], query_string.c_str(), query_string.length());
	close(pipefds[0]);
	close(pipefds[1]);
	waitpid(pid, 0, 0);
	return;
}

/*
	This function needs the first line of the http header and the body, so pass the full http message in full_message.
	root is the root of the server.
	output_fd is where the python script will write its ouput.
	The http message should be valid! (is_valid_for_cgi must have returned true with the same message/root)
 */
void execute_cgi(const std::string& full_message, std::string root, const std::string& location, int output_fd)
{
	if (root[root.length() -1] != '/') root += '/';
	using std::string;
	logn("Executing cgi...");
	const string request = get_next_word(full_message);
	string path = get_next_word(&full_message[request.length() + 1]);
	path = path.substr(location.length(), path.length() - location.length());
	std::size_t path_end = get_end_path(path, root);
	if (path_end == std::string::npos) path_end = path.length();
	std::string exec_path = root + path.substr(0, path_end);
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
		query_string = path.substr(qpos + 1, path.length() - qpos - 1);
		define_query = true;
	}
	else if (request == "POST")
	{
		const std::size_t content_length = atoi(get_header_info(full_message, "Content-Length").c_str());
		if (content_length != 0)
		{
			define_query = true;
			query_string = full_message.substr(full_message.find("\r\n\r\n") + 4, content_length);
		}
	}
	if (define_query == true)
		logn("QUERY_STRING==" + query_string);
	else
		logn("No QUERY_STRING will be defined/passed to stdin");
	setup_and_exec(output_fd, request, define_query, query_string, define_path, path_info, exec_path);
}



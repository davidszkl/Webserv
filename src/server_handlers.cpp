#include "webserver.hpp"

int	webserver::handle_GET(const pollfd &fd, server & server) {
	bool body					= true;
	std::string& response_file	= _http_request._path;
	const config::location& current_block = server._configs[_config_index].location_blocks[_location_index];
	struct stat s;

	if (std::find(current_block.allowed_methods.begin(), current_block.allowed_methods.end(),
	 	"GET") == current_block.allowed_methods.end())
	{
		_response_code = METHOD_NOT_ALLOWED;
		response_file = server._configs[_config_index].error_pages[METHOD_NOT_ALLOWED];
	}
	if (_http_request._path[_http_request._path.size() - 1] == '/')
		_http_request._path += current_block.index;
	logn("requestpath: " + _http_request._path);
    if (current_block.autoindex && stat(_http_request._path.c_str(), &s) == 0 && (s.st_mode & S_IFDIR))
    {
		logn("autoindexing from " + current_block.path);
		send_autoindex(fd);
		return 0;
    }
	else if (current_block.redirect != "")
	{
		_response_code = MOVED_PERMANENTLY;
		send_redirect(fd, current_block.redirect);
		return 0;
	}
	else if (!file_exists(_http_request._path)) {
		_response_code	= NOT_FOUND;
		response_file	= server._configs[_config_index].error_pages[NOT_FOUND];
	}
	else if (_http_request._path.find("server_files") == string::npos) {
		_response_code	= FORBIDDEN;
		response_file	= server._configs[_config_index].error_pages[FORBIDDEN];
	}
	else
		_response_code = OK;
	logn("RESPONSE_FILE\n" + response_file);
	send_response(fd, response_file, body);
	return 0;
}

void webserver::send_autoindex(const pollfd& fd) {
	string http_response;

	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	http_response += "\r\n\r\n";
	http_response += autoindex(_http_request._path);
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

void webserver::send_redirect(const pollfd& fd, const string& redirect)
{
	string http_response;

	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	http_response += "\r\n\r\n";
	http_response += ("Location: " + redirect);
	http_response += "\r\n\r\n";
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

void webserver::send_response(const pollfd &fd, string filename, bool body) {
	string http_response;
	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	http_response += "\r\n\r\n";
	if (body)
		http_response += slurp_file(filename);
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

int	webserver::handle_POST(const pollfd &fd, server &server) {
    bool body					= true;
    std::string& response_file	= _http_request._path;
    const config::location & current_block = server._configs[_config_index].location_blocks[_location_index];

    if (std::find(current_block.allowed_methods.begin(), current_block.allowed_methods.end(),"POST")== current_block.allowed_methods.end())
    {
        _response_code = METHOD_NOT_ALLOWED;
        response_file = server._configs[_config_index].error_pages[METHOD_NOT_ALLOWED];
    }
    if (_http_request._path[_http_request._path.size() - 1] == '/')
        _http_request._path += current_block.index;
    logn("requestpath: " + _http_request._path);
    if (!file_exists(_http_request._path)) {
        _response_code = NOT_FOUND;
        response_file = server._configs[_config_index].error_pages[NOT_FOUND];
    }
    else if (_http_request._path.find("server_files") == string::npos) {
        _response_code = FORBIDDEN;
        response_file = server._configs[_config_index].error_pages[FORBIDDEN];
    }
    else
        _response_code = OK;
    logn("RESPONSE_FILE\n" + response_file);
    send_response(fd, response_file, body);
    return 0;
}

int	webserver::handle_DELETE(const pollfd &fd, server& server) {
	bool body					= true;
	std::string& response_file	= _http_request._path;
	const config::location & current_block = server._configs[_config_index].location_blocks[_location_index];

	if (std::find(current_block.allowed_methods.begin(), current_block.allowed_methods.end(),
	 	"DELETE") == current_block.allowed_methods.end())
	{
		_response_code = METHOD_NOT_ALLOWED;
		response_file = server._configs[_config_index].error_pages[METHOD_NOT_ALLOWED];
	}
	if (!file_exists(_http_request._path)) {
		_response_code = NOT_FOUND;
		response_file = server._configs[_config_index].error_pages[NOT_FOUND];
	}
	else if (_http_request._path.find("server_files") == std::string::npos) {
		_response_code = FORBIDDEN;
		response_file = server._configs[_config_index].error_pages[FORBIDDEN];
	}
	else
	{
		body = false;
		if (remove(_http_request._path.c_str()))
			_response_code = FORBIDDEN;
		else
			_response_code = NO_CONTENT;			
	}
	send_response(fd, response_file, body);
	return 0;
}

inline bool file_exists (const string& name) {
    ifstream f(name.c_str());
    return f.good();
}
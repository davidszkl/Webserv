#pragma once
#include "server.hpp"
#include <vector>
#include <fstream>
#include <signal.h>									//TO_ERASE

enum RESPONSE_CODES {
	OK					= 200,
	CREATED				= 201,
	ACCEPTED			= 202,
	NO_CONTENT			= 204,
	RESET_CONTENT 		= 205,
	PARTIAL_CONTENT 	= 206,

	MULTIPLE_CHOICES	= 300,
	MOVED_PERMANENTLY	= 301,
	FOUND				= 302,
	SEE_OTHER			= 303,
	NOT_MODIFIED		= 304,
	USE_PROXY			= 305,
	TEMPORARY_REDIRECT	= 307,

	BAD_REQUEST						= 400,
	UNAUTHORIZED					= 401,
	FORBIDDEN						= 403,
	NOT_FOUND						= 404,
	METHOD_NOT_ALLOWED				= 405,
	NOT_ACCEPTABLE					= 406,
	PROXY_AUTHENTICATION_REQUIRED	= 407,
	REQUEST_TIMEOUT					= 408,
	CONFLICT						= 409,
	GONE							= 410,
	LENGTH_REQUIRED					= 411,
	PRECONDITION_FAILED				= 412,
	REQUEST_ENTITY_TOO_LARGE		= 413,
	REQUEST_URI_TOO_LONG			= 414,
	UNSUPPORTED_MEDIA_TYPE			= 415,
	REQUESTED_RANGE_NOT_SATISFIABLE	= 416,
	EXPECTATION_FAILED				= 417,

	INTERNAL_SERVER_ERROR			= 500,
	NOT_IMPLEMENTED					= 501,
	BAD_GATEWAY						= 502,
	SERVICE_UNAVAILABLE				= 503,
	GATEWAY_TIMEOUT					= 504,
	HTTP_VERSION_NOT_SUPPORTED		= 505
};

static bool				_server_alive;

enum METHODS {
	GET		= 1,
	POST	= 2,
	DELETE	= 3
};

class webserver
{
public:

	webserver() {};
	webserver(std::vector<int> config);
	~webserver();

	void	listen_all();
	int		clear_errors();
	int		get_fd_ready() const;
	int		read_msg(pollfd* fd);
	void	request_handler(const pollfd & fd);
	void	init_request();
	void	clear_request();
	int		handle_GET(const pollfd &fd);
	int		handle_POST(const pollfd &fd);
	int		handle_DELETE(const pollfd &fd);
	void	send_response(const pollfd &fd, std::string filename, bool error);
	std::string get_code_description(int code);

	class webserver_exception : public std::runtime_error
	{
	public:
		webserver_exception(const char* what): runtime_error(what) {}
	};

	struct http_request
	{
		std::string	_full_request;
		std::string	_header;
		std::string	_body;
		std::string _method;
		std::string _uri;
		std::string _version;
		std::string _path;
		std::vector<std::string> _header_lines;
	};
	
private:

	std::vector<server>	_servers;
	std::vector<pollfd>	_pollsock;
	sockaddr_in			_client_addr;
	pollfd				_pollfd[1];
	socklen_t			_socklen;
	http_request		_http_request;
	std::string			_root;
	int					_response_code;
};

void 		poll_result(const pollfd & fd);
std::string my_get_line(std::string from );
std::string i_to_str(int nbr);
std::string slurp_file(std::string file);
std::string read_header_line(std::string from);
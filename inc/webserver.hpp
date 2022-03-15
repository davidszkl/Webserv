#pragma once
#include "server.hpp"
#include <fstream>
#include "cgi.hpp"
#include <signal.h>									//TO_ERASE


enum RESPONSE_CODES {
	OK					= 200,	//Martin
	CREATED				= 201,	//Martin
	ACCEPTED			= 202,	//Martin
	NO_CONTENT			= 204,	//Martin
	RESET_CONTENT 		= 205,	//Martin
	PARTIAL_CONTENT 	= 206,	//Martin

	MULTIPLE_CHOICES	= 300,  //X
	MOVED_PERMANENTLY	= 301,	//Martin
	FOUND				= 302,	//X
	SEE_OTHER			= 303,	//X
	NOT_MODIFIED		= 304,	//X
	USE_PROXY			= 305,	//X
	TEMPORARY_REDIRECT	= 307,	//X

	BAD_REQUEST						= 400,	//OK
	UNAUTHORIZED					= 401,	//X
	FORBIDDEN						= 403,	//GET: OK,
	NOT_FOUND						= 404,	//GET: OK,
	METHOD_NOT_ALLOWED				= 405,	//GET: OK,
	NOT_ACCEPTABLE					= 406,	//X
	PROXY_AUTHENTICATION_REQUIRED	= 407,	//X
	REQUEST_TIMEOUT					= 408,	//X
	CONFLICT						= 409,	//X
	GONE							= 410,	//X
	LENGTH_REQUIRED					= 411,	//Martin
	PRECONDITION_FAILED				= 412,	//X
	REQUEST_ENTITY_TOO_LARGE		= 413,	//Martin
	REQUEST_URI_TOO_LONG			= 414,	//OK
	UNSUPPORTED_MEDIA_TYPE			= 415,	//X
	REQUESTED_RANGE_NOT_SATISFIABLE	= 416,	//X
	EXPECTATION_FAILED				= 417,	//X

	INTERNAL_SERVER_ERROR			= 500,	//X
	NOT_IMPLEMENTED					= 501,	//OK
	BAD_GATEWAY						= 502,	//X
	SERVICE_UNAVAILABLE				= 503,	//X
	GATEWAY_TIMEOUT					= 504,	//X
	HTTP_VERSION_NOT_SUPPORTED		= 505	//OK
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
	void	clear_errors();
	void	init_pollsock();
	int		get_fd_ready()			const;
	int		get_server_id(int fd)	const;
	int		read_msg(int fd);
	void	request_handler(const pollfd & fd, server & server);
	void	init_request();
	void	clear_request();
	int		handle_GET(const pollfd &fd, server & server);
	int		handle_POST(const pollfd &fd);
	int		handle_DELETE(const pollfd &fd, server & server);
	void	send_response(const pollfd &fd, std::string filename, bool error);
	bool	is_deletable(server & server, const std::string& filename) const;
	std::string get_code_description(int code) const;

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
	pollfd				_pollfd;
	socklen_t			_socklen;
	http_request		_http_request;
	std::string			_root;
	int					_response_code;
	size_t				_content_length;
};

void 		poll_result(const pollfd & fd);
bool		find_crlf(std::string str);
bool		is_post(std::string str);
inline bool file_exists (const std::string& name);
std::string my_get_line(std::string from );
std::string i_to_str(int nbr);
std::string slurp_file(std::string file);
std::string read_header_line(std::string from);
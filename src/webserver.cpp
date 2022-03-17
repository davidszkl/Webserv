#include "webserver.hpp"
#include "debug.hpp"

bool _server_alive = true;

//TO_ERASE
void server_shutdown(int signbr) {
	(void)signbr;
	_server_alive = false;
}
//TO_ERASE

webserver::webserver(std::vector<int> config):	_socklen(sizeof(_client_addr)), _root("server_files"), _response_code(404)
{
	signal(SIGINT, &server_shutdown);
	_server_alive = true;
	memset(&_pollfd, 0, sizeof(_pollfd));
	_pollfd.fd	= -1;
	try {
		_servers.reserve(config.size());				//don't call all destructors every time a server is added
		for (size_t n = 0; n < config.size(); n++)
			_servers.push_back(server(config[n]));		//see ~webserv()
	}
	catch (...) {
		throw ;
	}
}

webserver::~webserver() {
	cerr << "webserv destructor" << endl;
	for (size_t n = 0; n < _servers.size(); n++) {		//~webserv() handles closing of server sockets because
		if (_servers[n]._sockfd)
		{
			cerr << "CLOSING FD " << _servers[n]._sockfd << endl;					//vector calls server destructor all the time
			close(_servers[n]._sockfd);
		}
	}
}

void webserver::init_pollsock()
{
	pollfd listener;
	memset(&listener, 0, sizeof(listener));
	_pollsock.push_back(listener);
	for (size_t n = 0; n < _servers.size(); n++)		//initialize vector of pollfd that contains all server-sockets
	{
		pollfd tmp;
		memset(&tmp, 0, sizeof(tmp));
		tmp.fd		= _servers[n]._sockfd;
		tmp.events	= POLLIN | POLLOUT;
		_pollsock.push_back(tmp);
	}
}

int webserver::get_fd_ready() const {					//get the first open fd out of poll()
	for (size_t n = 0; n < _pollsock.size(); n++)
		if (_pollsock[n].revents & POLLIN)
			return _pollsock[n].fd;
	return -1;
}

void webserver::listen_all()
{
	init_pollsock();
	int accept_fd = -1;
	while (_server_alive)
	{
		int rval = 0;
		while (true)
		{
			cerr << "Waiting for connection.\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)))
				break ;
			cerr << "Waiting for connection..\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)))
				break ;
			cerr << "Waiting for connection...\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)))
				break ;
			cerr << "                         \r";
		}
		cerr << endl;
		if (rval < 0)
			throw webserver_exception("Poll failed on an fd");

		if (accept_fd < 0)
		{
			accept_fd = get_fd_ready();
			cerr << "accept_fd " << accept_fd << endl;
			if ((_pollsock[0].fd = accept(accept_fd,								\
										reinterpret_cast<sockaddr*>(&_client_addr),	\
										&_socklen))									\
										< 0)
				throw webserver_exception("Accept failed");
			_pollsock[0].events = POLLIN | POLLOUT;
			cerr << "Connection on fd " << accept_fd << " accepted" << endl \
				 << "Connection fd is " << _pollsock[0].fd << endl;
			continue ;
		}

		if (!(_pollsock[0].revents & POLLIN))
			continue ;
		int read_rval = read_msg(_pollsock[0].fd);
		if (read_rval == -1) {
			cerr << "Fatal problem occured during connection with " << accept_fd << endl;
			cerr << "ERRNO " << errno << endl;
			throw webserver_exception("Poll fatal error");
		}
		try {
			int id = get_server_id(accept_fd);
			request_handler(_pollsock[0], _servers[id]);
		}
		catch (webserver_exception & e) {
			cerr << e.what() << endl;
		}
		close(_pollsock[0].fd);
		_pollsock[0].fd		= 0;
		_pollsock[0].events = 0;
		accept_fd = -1;
		clear_errors();
	}
	cerr << "Stop message received.\nShutting down server." << endl;
}

int webserver::read_msg(int fd) {;
	char buffer[100] = {0};
	int end = 0;
	clear_request();
	cerr << "Receiving message:\n";
	while(!find_crlf(std::string(buffer)))
	{
		end = recv(fd, &buffer, 100, 0);
		if (end < 0)
			return -1;
		buffer[end] = '\0';
		_http_request._full_request += buffer;
	}
	if (is_post(_http_request._full_request))
	{
		_content_length = std::atoi(get_header_info(_http_request._full_request, "Content-Length").c_str());
		while(!find_crlf(std::string(buffer)))
		{
			end = recv(fd, &buffer, 100, 0);
			if (end < 0)
				return -1;
			buffer[end] = '\0';
			_http_request._full_request += buffer;
		}
	}
	cerr << "Quitting\n";
	return 0;
}

bool find_crlf(std::string str) {
	if (str.find("\r\n\r\n") != std::string::npos)
		return true;
	return false;
}

bool is_post(std::string str) {
	if (str.find("POST") != std::string::npos)
		return true;
	return false;
}

void webserver::request_handler(const pollfd & fd, server & server) {
	init_request();
	if (!_http_request._method.size() || !_http_request._uri.size() || !_http_request._version.size()) {
		_response_code = BAD_REQUEST;
		send_response(fd, "", false);
	}
	else if (_http_request._uri.size() > 1024)
	{
		_response_code = REQUEST_URI_TOO_LONG;
		send_response(fd, "", false);
	}
	else if (_http_request._version != "HTTP/1.1")
	{
		_response_code = HTTP_VERSION_NOT_SUPPORTED;
		send_response(fd, "", false);
	}
	else if (_http_request._method	== "GET")
		handle_GET(fd, server);
	else if (_http_request._method	== "POST")
		handle_POST(fd);
	else if (_http_request._method	== "DELETE")
		handle_DELETE(fd);
	else {
		_response_code = NOT_IMPLEMENTED;
		send_response(fd, server._error_pages[NOT_IMPLEMENTED], true);
	}
}

int	webserver::handle_GET(const pollfd &fd, server & server) {
	bool body					= true;
	std::string& response_file	= _http_request._path;

	if (std::find	(server._allowed_methods.begin(), server._allowed_methods.end(), "GET") == server._allowed_methods.end())
	{
		_response_code = METHOD_NOT_ALLOWED;
		response_file = server._error_pages[METHOD_NOT_ALLOWED];
	}
	else if (!file_exists(_http_request._path)) {
		_response_code = NOT_FOUND;
		response_file = server._error_pages[NOT_FOUND];
	}
	else if (_http_request._path.find("server_files") == std::string::npos) {
		_response_code = FORBIDDEN;
		response_file = server._error_pages[FORBIDDEN];
	}
	else
		_response_code = OK;
	send_response(fd, response_file, body);
	return 0;
}

int	webserver::handle_POST(const pollfd &fd) {
	cerr << "POST handler for " << fd.fd << endl;
	//CODE 201
	return 0;
}

int	webserver::handle_DELETE(const pollfd &fd) {
	cerr << "DELETE handler for " << fd.fd << endl;
	return 0;
}

void webserver::send_response(const pollfd &fd, std::string filename, bool body) {
	std::string http_response;
	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	if (body) {
		http_response += "\r\n\r\n";
		http_response += slurp_file(filename);
	}
	if (filename != "favicon.ico")
		cerr << "RESPONSE\n" << http_response << endl; 
	else
		cerr << "RESPONSE IS FAVICON" << endl;
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

void webserver::init_request() {

	int temp_pos = _http_request._full_request.find("\r\n\r\n");
	if (temp_pos > 0)
		_http_request._header = _http_request._full_request.substr(0, temp_pos);
	_http_request._body = _http_request._full_request.substr(temp_pos + 4, _http_request._full_request.size());

	size_t cursor = 0;
	while(1)
	{
		std::string str_from(&_http_request._header[cursor]);
		std::string line = read_header_line(&_http_request._header[cursor]);
		cursor += line.size();
		_http_request._header_lines.push_back(line);
		if (line == "\r\n" || !line.size())
			break ;
	}
	std::stringstream ss(_http_request._header_lines[0]);
	ss >> _http_request._method;
	ss >> _http_request._uri;
	_http_request._path = _root + _http_request._uri;
	cerr << "URI\n" << _http_request._uri << endl;
	cerr << "PATH\n" << _http_request._path << endl;
	ss >> _http_request._version;
	cerr << "_http_request._header:\n"	<< _http_request._header << endl;
}

void webserver::clear_errors() {						//clear servers that got shutdown for some reason
	for (size_t n = 1; n < _pollsock.size(); n++)
	{
		if (_pollsock[n].revents & POLLERR)
		{
			cerr << "killing server " << n << endl;
			close(_pollsock[n].fd);
			_servers.erase(_servers.begin() + n);
			_pollsock.erase(_pollsock.begin() + n);
		}
	}
}

// bool find_end_of_body(std::string str) {

// }

inline bool file_exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

std::string webserver::get_code_description(int code) const {
	switch (code) {
		case OK :
			return " OK";
		case CREATED :
			return " CREATED";
		case ACCEPTED :
			return " ACCEPTED";
		case NO_CONTENT :
			return " NO_CONTENT";
		case RESET_CONTENT :
			return " RESET_CONTENT";
		case PARTIAL_CONTENT :
			return " PARTIAL_CONTENT";
		case MULTIPLE_CHOICES :
			return " MULTIPLE_CHOICES";
		case MOVED_PERMANENTLY :
			return " MOVED_PERMANENTLY";
		case FOUND :
			return " FOUND";
		case SEE_OTHER :
			return " SEE_OTHER";
		case NOT_MODIFIED :
			return " NOT_MODIFIED";
		case USE_PROXY :
			return " USE_PROXY";
		case TEMPORARY_REDIRECT :
			return " TEMPORARY_REDIRECT";
		case BAD_REQUEST :
			return " BAD_REQUEST";
		case UNAUTHORIZED :
			return " UNAUTHORIZED";
		case FORBIDDEN :
			return " FORBIDDEN";
		case NOT_FOUND :
			return " NOT_FOUND";
		case METHOD_NOT_ALLOWED :
			return " METHOD_NOT_ALLOWED";
		case NOT_ACCEPTABLE :
			return " NOT_ACCEPTABLE";
		case PROXY_AUTHENTICATION_REQUIRED :
			return " PROXY_AUTHENTICATION_REQUIRED";
		case REQUEST_TIMEOUT :
			return " REQUEST_TIMEOUT";
		case CONFLICT :
			return " CONFLICT";
		case GONE :
			return " GONE";
		case LENGTH_REQUIRED :
			return " LENGTH_REQUIRED";
		case PRECONDITION_FAILED :
			return " PRECONDITION_FAILED";
		case REQUEST_ENTITY_TOO_LARGE :
			return " REQUEST_ENTITY_TOO_LARGE";
		case REQUEST_URI_TOO_LONG :
			return " REQUEST_URI_TOO_LONG";
		case UNSUPPORTED_MEDIA_TYPE :
			return " UNSUPPORTED_MEDIA_TYPE";
		case REQUESTED_RANGE_NOT_SATISFIABLE :
			return " REQUESTED_RANGE_NOT_SATISFIABLE";
		case EXPECTATION_FAILED :
			return " EXPECTATION_FAILED";
		case INTERNAL_SERVER_ERROR :
			return " INTERNAL_SERVER_ERROR";
		case NOT_IMPLEMENTED :
			return " NOT_IMPLEMENTED";
		case BAD_GATEWAY :
			return " BAD_GATEWAY";
		case SERVICE_UNAVAILABLE :
			return " SERVICE_UNAVAILABLE";
		case GATEWAY_TIMEOUT :
			return " GATEWAY_TIMEOUT";
		default :
			return " ";
	}
}

std::string slurp_file(std::string file) {
	std::ifstream stream(file.c_str());
	std::stringstream buffer;
	buffer << stream.rdbuf();
	std::string file_content(buffer.str());
	if (!(file == "server_files/favicon.ico"))
	{
		cout << "WEBPAGE\n" << file_content << endl \
 			 << "WEBPAGE " << endl;
	}
	return file_content;
}

std::string i_to_str(int nbr) {
	std::stringstream ss;
    std::string s;
    ss << nbr;
    s = ss.str();

    return s;
}

std::string my_get_line(std::string from ) {
	std::string to;
	for (size_t n = 0; from[n] && from[n] != '\n'; n++)
		to += from[n];
	return to;
}

std::string read_header_line(std::string from) {
	std::string str;
	for (size_t n = 0; from[n]; n++)
	{
		str += from[n];
		if (str[n] == '\n' && str.size() > 2 && str[n - 1] == '\r')
			break;
	}
	return str;
}

void poll_result(const pollfd & fd){
	cout << "events  : "												\
		 << (fd.events & POLLIN 	? " POLLIN |"	: "        |")		\
		 << (fd.events & POLLHUP	? " POLLHUP |"	: "         |")		\
		 << (fd.events & POLLERR	? " POLLERR |"	: "         |")		\
		 << (fd.events & POLLPRI	? " POLLPRI |"	: "         |")		\
		 << (fd.events & POLLOUT	? " POLLOUT |"	: "         |")		\
		 << (fd.events & POLLNVAL	? " POLLNVAL |"	: "          |")	<< endl;
	cout << "revents : " 												\
		 << (fd.revents & POLLIN 	? " POLLIN |"	: "        |")		\
		 << (fd.revents & POLLHUP	? " POLLHUP |"	: "         |")		\
		 << (fd.revents & POLLERR	? " POLLERR |"	: "         |")		\
		 << (fd.revents & POLLPRI	? " POLLPRI |"	: "         |")		\
		 << (fd.revents & POLLOUT	? " POLLOUT |"	: "         |")		\
		 << (fd.revents & POLLNVAL	? " POLLNVAL |"	: "          |")	<< endl;
}

void	webserver::clear_request() {
	_http_request._full_request.clear();
	_http_request._header.clear();
	_http_request._body.clear();
	_http_request._method.clear();
	_http_request._uri.clear();
	_http_request._version.clear();
	_http_request._header_lines.clear();
}

int webserver::get_server_id(int fd_tofind) const {
	for (size_t n = 1; n < _pollsock.size(); n++) {
		if (_pollsock[n].fd == fd_tofind)
			return n - 1;
	}
	return -1;
}

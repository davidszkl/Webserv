#include "webserver.hpp"

//TO_ERASE
void server_shutdown(int signbr) {
	(void)signbr;
	_server_alive = false;
}
//TO_ERASE

webserver::webserver(std::vector<int> config):	_socklen(sizeof(_client_addr)), _response_code(404)
{
	signal(SIGINT, &server_shutdown);
	_server_alive = true;
	memset(&_pollfd, 0, sizeof(_pollfd));
	_pollfd[0].events	= POLLIN | POLLOUT;
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
	if (_pollfd[0].fd)
	{
		cerr << "CLOSING POLLFD\n";
		close(_pollfd[0].fd);
	}
}

int webserver::clear_errors() {						//clear servers that got shutdown for some reason
	for (size_t n = 0; n < _pollsock.size(); n++)
	{
		if (_pollsock[n].revents & POLLERR || \
			_pollsock[n].revents & POLLHUP )
		{
			_servers.erase(_servers.begin() + n);
			_pollsock.erase(_pollsock.begin() + n);
			if (close(_pollsock[n].fd) < 0)
				return -1;
		}
	}
	return 0;
}

int webserver::get_fd_ready() const {					//get the first open fd out of poll()
	for (size_t n = 0; n < _pollsock.size(); n++)
		if (_pollsock[n].revents & POLLIN && \
			!(_pollsock[n].revents & POLLERR) )
			return _pollsock[n].fd;
	return -1;
}

void webserver::listen_all()
{
	for (size_t n = 0; n < _servers.size(); n++)		//initialize vector of pollfd that contains all server-sockets
	{
		pollfd tmp;
		memset(&tmp, 0, sizeof(tmp));
		tmp.fd		= _servers[n]._sockfd;
		tmp.events	= POLLIN | POLLOUT;
		_pollsock.push_back(tmp);
	}

	while (_server_alive)
	{
		int rval = 0;
		while (!rval)
		{
			cerr << "Waiting for connection.\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)) != 0)
				break ;
			cerr << "Waiting for connection..\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)) != 0)
				break ;
			cerr << "Waiting for connection...\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)) != 0)
				break ;
			cerr << "                         \r";
		}
		cerr << endl;
		if (rval < 0)
			throw webserver_exception("Poll failed on an fd");

		int accept_fd = get_fd_ready();
		if ((_pollfd[0].fd = accept(accept_fd,								\
									reinterpret_cast<sockaddr*>(&_client_addr),	\
									&_socklen))									\
									< 0)
			throw webserver_exception("Accept failed");
		cerr << "Connection on fd " << accept_fd << " accepted" << endl \
			 << "Connection fd is " << _pollfd[0].fd << endl;

		int read_rval = read_msg(_pollfd);
		if (read_rval == -1) {
			cerr << "Fatal problem occured during connection with " << accept_fd << endl;
			cerr << "ERRNO " << errno << endl;
			throw webserver_exception("Poll fatal error");
		}
		if (read_rval == -2) {
			cerr << "Problem occured during connection with " << accept_fd << endl;
		}
		if (read_rval == -3) {
			cerr << "Problem occured with recv() " << accept_fd << endl;
		}
		try {
			request_handler(_pollfd[0]);
		}
		catch (webserver_exception & e) {
			cerr << e.what() << endl			\
				 << "Short request: " << endl	\
				 << _short_request << endl;
			send_error_code(_pollfd[0]);
		}
		if (close(_pollfd[0].fd) < 0)
			throw webserver_exception("Could not close connection fd");
		if (clear_errors() < 0)
			throw webserver_exception("Could not clear bad fd");
	}
	cerr << "Stop message received.\nShutting down server." << endl;
}

void webserver::request_handler(const pollfd & fd) {
	_short_request = my_get_line(_request);
	init_request();
	if (_http_request._uri == "/stop") {
		_server_alive = false;
		return ;
	}
	if (!_http_request._method.size() || !_http_request._uri.size() || !_http_request._version.size()) {
		_response_code = BAD_REQUEST;
		throw webserver_exception("Request line incomplete");
	}
	if (_http_request._method		== "GET")
		handle_GET(fd);
	else if (_http_request._method	== "POST")
		handle_POST(fd);
	else if (_http_request._method	== "DELETE")
		handle_DELETE(fd); 
	else {
		_response_code = NOT_IMPLEMENTED;
		throw webserver_exception("Unknown method");
	}
	return ;
}

void webserver::init_request() {
	std::stringstream ss(_short_request);
	ss >> _http_request._method;
	cerr << "_http_request._method: " << _http_request._method << endl;
	ss >> _http_request._uri;
	cerr << "_http_request._uri: " << _http_request._uri << endl;
	ss >> _http_request._version;
	cerr << "_http_request._version: " << _http_request._version << endl;
}

void poll_result(const pollfd & fd) {
	cout << "events  : "												\
		 << (fd.events & POLLIN 	? " POLLIN |"		: "        |")	\
		 << (fd.events & POLLHUP	? " POLLHUP |"	: "         |")		\
		 << (fd.events & POLLERR	? " POLLERR |"	: "         |")		\
		 << (fd.events & POLLPRI	? " POLLPRI |"	: "         |")		\
		 << (fd.events & POLLOUT	? " POLLOUT |"	: "         |")		\
		 << (fd.events & POLLNVAL	? " POLLNVAL |"	: "          |")	<< endl;
	cout << "revents : " 												\
		 << (fd.revents & POLLIN 	? " POLLIN |"		: "        |")	\
		 << (fd.revents & POLLHUP	? " POLLHUP |"	: "         |")		\
		 << (fd.revents & POLLERR	? " POLLERR |"	: "         |")		\
		 << (fd.revents & POLLPRI	? " POLLPRI |"	: "         |")		\
		 << (fd.revents & POLLOUT	? " POLLOUT |"	: "         |")		\
		 << (fd.revents & POLLNVAL	? " POLLNVAL |"	: "          |")	<< endl;
}

std::string my_get_line(std::string from ) {
	std::string to;
	for (size_t n = 0; from[n] && from[n] != '\n'; n++)
		to += from[n];
	return to;
}

int	webserver::handle_GET(const pollfd &fd) {
	if (_http_request._uri == "/" ||
		_http_request._uri == "/index.html")
		send_response(fd);
	send_error_code(fd);
	return 0;
}

int	webserver::handle_POST(const pollfd &fd) {
	cerr << "POST handler for " << fd.fd << endl;
	return 0;
}

int	webserver::handle_DELETE(const pollfd &fd) {
	cerr << "DELETE handler for " << fd.fd << endl;
	return 0;
}

void webserver::send_error_code(const pollfd &fd) {
	std::string http_response;
	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

std::string slurp_file(std::string file) {
	std::ifstream stream(file);
	std::stringstream buffer;
	buffer << stream.rdbuf();
	std::string file_content(buffer.str());
	cout << "WEBPAGE " << file_content << endl \
 		 << "WEBPAGE " << endl;
	return file_content;
}

void webserver::send_response(const pollfd &fd) {
	std::string http_response;
	_response_code = 200;
	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	http_response += "\r\n\r\n";
	http_response += slurp_file("index.html");
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

int webserver::read_msg(pollfd* fd) {
	_request.clear();
	while (true)
	{
		char buffer[100];
		int rval = 0;
		while (!rval)
		{
			rval = poll(fd, 1, 1000);
			//poll_result(fd[0]);
		}
		if (rval < 0)
			return -1;
		if (fd[0].revents & POLLERR || 							\
			(fd[0].revents & POLLHUP && !(fd[0].revents & POLLIN)))
			return -2;
		if (fd[0].revents & POLLIN)
		{
			cerr << "receiving message:\n";
			int end = recv(fd[0].fd, &buffer, 100, 0);
			if (end < 0)
				return -3;
			else if (!end) {
				cerr << "Received nothing.\n";
				break;
			}
			buffer[end] = '\0';
			_request += buffer;
			if (buffer[end - 1] == '\n' && buffer[end - 2] == '\r')
				break;
		}
	}
	if (_request == "stop")
		_server_alive = false;
	cerr << "Quitting\n";
	return 0;
}

std::string i_to_str(int nbr) {
	std::stringstream ss;
    std::string s;
    ss << nbr;
    s = ss.str();

    return s;
}

std::string webserver::get_code_description(int code) {
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

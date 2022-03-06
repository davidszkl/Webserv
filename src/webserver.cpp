#include "webserver.hpp"

webserver::webserver(std::vector<int> config):	_socklen(sizeof(_client_addr)),
												_server_alive(true) {
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
			throw webserver_exception("Poll fatal error");
		}
		if (read_rval == -2) {
			cerr << "Problem occured during connection with " << accept_fd << endl;
		}
		if (request_handler(_pollfd[0]) < 0)
		{
			cerr << "Request could not be handled" << endl	\
				 << "Short request: " << endl				\
				 << _short_request << endl;
		}
		if (close(_pollfd[0].fd) < 0)
			throw webserver_exception("Could not close connection fd");
		if (clear_errors() < 0)
			throw webserver_exception("Could not clear bad fd");
	}
	cerr << "Stop message received.\nShutting down server." << endl;
}

int webserver::read_msg(pollfd* fd) {
	_request.clear();
	while (true)
	{
		char buffer[100];
		int rval = 0;
		while (!rval)
			rval = poll(fd, 1, 1000);
		poll_result(fd[0]);
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
				return -1;
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

int	webserver::request_handler(const pollfd & fd) {
	_short_request = my_get_line(_request);
	init_request();
	if (_http_request._uri == "/stop") {
		_server_alive = false;
		return 0;
	}
	//size_t n = 0;
	// for (; _http_request._method[n]; n++)
	// 	cerr << _http_request._method[n] << " " << (int)_http_request._method[n] << endl;
	// cerr << _http_request._method[n] << (int)_http_request._method[n] << endl;
	cerr << (_http_request._method == "GET") << endl;
	cerr << (_http_request._method == std::string("GET")) << endl;
	if (!strcmp(_http_request._method.c_str(), "GET"))
		handle_GET(fd);
	else if (!strcmp(_http_request._method.c_str(), "POST"))
		handle_POST(fd);
	else if (!strcmp(_http_request._method.c_str(), "DELETE"))
		handle_DELETE(fd); 
	else
		return -1;
	return 0;
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
	cout << "events : "
		 << (fd.revents & POLLIN 	? "POLLIN"	 : "")	<< " "	\
		 << (fd.revents & POLLHUP	? "POLLHUP"	 : "")	<< " "	\
		 << (fd.revents & POLLERR	? "POLLERR"	 : "")	<< " "	\
		 << (fd.revents & POLLPRI	? "POLLPRI"	 : "")	<< " "	\
		 << (fd.revents & POLLOUT	? "POLLOUT"	 : "")	<< " "	\
		 << (fd.revents & POLLNVAL	? "POLLNVAL" : "")	<< endl;
}

std::string my_get_line(std::string from ) {
	std::string to;
	for (size_t n = 0; from[n] && from[n] != '\n'; n++)
		to += from[n];
	return to;
}

int		webserver::handle_GET(const pollfd &fd) {
	cerr << "GET handler for " << fd.fd << endl;
	return 0;
}

int		webserver::handle_POST(const pollfd &fd) {
	cerr << "POST handler for " << fd.fd << endl;
	return 0;
}

int		webserver::handle_DELETE(const pollfd &fd) {
	cerr << "DELETE handler for " << fd.fd << endl;
	return 0;
}
#include "webserver.hpp"

webserver::webserver(size_t servers_count) {
	_servers.reserve(servers_count);
}

webserver::~webserver() {
	cerr << "webserv destructor" << endl;
	for (size_t n = 0; n < _servers.size(); n++) {
		cout << "N = " << n << endl;
		if (_servers[n]._sockfd)
			close(_servers[n]._sockfd);
	}
}

void webserver::add_server(int port)
{
	try {
		_servers.push_back(server(port));
	}
	catch (server::server_error& e) {
		cerr << e.what() << endl;
	}
}

void webserver::clear_errors() {
	for (size_t n = 0; n < _pollsock.size(); n++)
	{
		if (_pollsock[n].revents & POLLNVAL)
		{
			if (poll(&_pollsock[n], 1, 0))
				close(_servers[n]._sockfd);
			_servers.erase(_servers.begin() + n);
			_pollsock.erase(_pollsock.begin() + n);
		}
	}
}

int webserver::get_fd_ready() const {
	for (size_t n = 0; n < _pollsock.size(); n++)
		if (_pollsock[n].revents & POLLIN)
			return _pollsock[n].fd;
	return -1;
}

void webserver::listen_all()
{
	//initialize vector of pollfd that contains all server-sockets
	for (size_t n = 0; n < _servers.size(); n++)
	{
		pollfd tmp;
		memset(&tmp, 0, sizeof(tmp));
		tmp.fd		= _servers[n]._sockfd;
		tmp.events	= POLLIN;
		_pollsock.push_back(tmp);
	}
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
		throw webserver_exception("poll failed on an fd");
	clear_errors();

	int accept_fd = get_fd_ready();
	_socklen = sizeof(_client_addr);
	if ((_pollfd.fd = accept(	accept_fd,									\
								reinterpret_cast<sockaddr*>(&_client_addr),	\
								&_socklen))									\
								< 0)
		throw webserver_exception("accept failed");
	cerr << "connection on fd " << accept_fd << " accepted" << endl \
		 << "client fd is " << _pollfd.fd << endl; 
}
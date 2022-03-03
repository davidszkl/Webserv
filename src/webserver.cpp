#include "webserver.hpp"

webserver::webserver(size_t servers_count) {
	_servers.reserve(servers_count);
}

webserver::~webserver() {
	for (size_t n = 0; n < _servers.size(); n++) {
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
	for (size_t n = 0; n < _pollsock.size(); n++)
		cerr << _pollsock[n].revents << endl;
	//if ((_pollfd = accept()) )
	(void)_pollfd;
	(void)_client_addr;
}
#include "webserver.hpp"

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
		cout << "TEST " << _servers[n]._sockfd<< endl;
		cout << "TEST " << _servers[n]._port << endl;

		pollfd tmp;
		memset(&tmp, 0, sizeof(tmp));
		tmp.fd		= _servers[n]._sockfd;
		tmp.events	= POLLIN;
		_pollsock.push_back(tmp);
	}
	for (size_t n = 0; n < _pollsock.size(); n++) {
		cout << _pollsock[n].fd << endl;
	}
	int rval = -1;
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
	{
		cerr << "ERROR " << errno << endl;
		throw webserver_exception("poll failed on an fd");
	}
	(void)_pollfd;
	(void)_client_addr;
}
#pragma once
#include "server.hpp"

class webserver
{
public:

	webserver()  {};
	~webserver() {};

	void add_server(int port);
	void listen_all();

	class webserver_exception : public std::runtime_error
	{
	public:
		webserver_exception(const char* what): runtime_error(what) {}
	};

//private:

	std::vector<server> _servers;
	std::vector<pollfd> _pollsock;
	pollfd				_pollfd;
	sockaddr_in			_client_addr;
};

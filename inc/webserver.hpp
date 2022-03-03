#pragma once
#include "server.hpp"
#include <vector>

class webserver
{
public:

	webserver() {};
	webserver(size_t servers_count);
	~webserver();

	void	add_server(int port);
	void	listen_all();
	void	clear_errors();
	int		get_fd_ready() const;

	class webserver_exception : public std::runtime_error
	{
	public:
		webserver_exception(const char* what): runtime_error(what) {}
	};

//private:

	std::vector<server>	_servers;
	std::vector<pollfd>	_pollsock;
	pollfd				_pollfd;
	socklen_t			_socklen;
	sockaddr_in			_client_addr;
};

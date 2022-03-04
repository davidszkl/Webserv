#pragma once
#include "server.hpp"
#include <vector>

class webserver
{
public:

	webserver() {};
	webserver(std::vector<int> config);
	~webserver();

	void	listen_all();
	void	clear_errors();
	int		get_fd_ready() const;
	int		read_msg(pollfd* fd);

	class webserver_exception : public std::runtime_error
	{
	public:
		webserver_exception(const char* what): runtime_error(what) {}
	};

//private:

	std::vector<server>	_servers;
	std::vector<pollfd>	_pollsock;
	std::string			_request;
	sockaddr_in			_client_addr;
	pollfd				_pollfd[1];
	socklen_t			_socklen;
	bool				_server_alive;
};

#pragma once
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netdb.h>

#include <cstring>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>				//close()
//#include <errno.h>

#include <arpa/inet.h>			//inet_addr()
#include <netinet/in.h>			//sockaddr_in
#include <poll.h>				//poll()
#include <vector>

#define MAX_ACCEPT_QUEUE 128	//size of the queue for the socket

using std::cerr;
using std::cout;
using std::endl;

class server
{
public:

	server(const int port);
	~server();

	class server_exception : public std::runtime_error
	{
	public:
		server_exception(const char* what) : runtime_error(what) {}
	};

	class location {
		friend class server;
		friend class webserver;
		std::string _path;			// contains location path last '/' will be removed
		std::string _root;			// empty if none
		std::string _index;			//empty if none
		std::string _redirect;		// empty if no redirect
		std::string _upload_dir;	//empty if upload not permitted
		std::vector<std::string> _allowed_methods;
		bool		_autoindex;
		location() {_autoindex = false;};
	};

	typedef std::map<const int, std::string>::iterator map_it;
private:
	std::string server_name;
	sockaddr_in		_server_addr;
	int				_sockfd;
	unsigned short	_port;
	size_t			_max_body;
	std::map<const int, std::string>	_error_pages;
	std::vector<location>				_location_blocks;

	friend class webserver;
};
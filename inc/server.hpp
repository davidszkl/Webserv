#pragma once
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netdb.h>

#include <cstring>

#include <stdexcept>
#include <iostream>
#include <sstream>

#include <fcntl.h>
#include <unistd.h>				//close()
//#include <errno.h>

#include <arpa/inet.h>			//inet_addr()
#include <netinet/in.h>			//sockaddr_in
#include <poll.h>				//poll()

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

	void test_alive() const;
private:

	friend class webserver;
	
	sockaddr_in		_server_addr;
	int				_sockfd;
	int				_port;
	std::string		_401_page;
	std::string		_403_page;
	std::string		_404_page;
	std::string		_405_page;
	std::string		_501_page;
	std::string		_503_page;
};

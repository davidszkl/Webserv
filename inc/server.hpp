#pragma once
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netdb.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <utility>

#include <fcntl.h>
#include <unistd.h>				//close()
#include <errno.h>

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

	class server_error : public std::runtime_error
	{
	public:
		server_error(const char* what) : runtime_error(what) {}
	};

	void test_alive() const;
private:

	friend class webserver;
	
	sockaddr_in		_server_addr;
	sockaddr_in 	_client_addr;
	socklen_t		_socket_len;
	int				_sockfd;
	int				_poll_rval;
	int				_port;
};
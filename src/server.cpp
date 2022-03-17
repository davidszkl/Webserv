#include "server.hpp"

server::server(config conf):
	_port(conf.port),
	_max_body(conf.max_body),
	_server_name(conf.server_name),
	_error_pages(conf.error_pages),
	_location_blocks(conf.location_blocks)
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
		throw server_exception("Socket creation failed.\nShutting down server.\n");
	cerr << "Socket " << _sockfd << " Successfully created" << endl;

	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_sockfd);
		throw server_exception("Socket configuration failed. ( fcntl() )\nShutting down server.\n");
	}
	cerr << "O_NONBLOCK set" << endl;

	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family			= AF_INET;
	_server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	_server_addr.sin_port			= htons(_port);

	int temporary = 0;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &temporary, sizeof(int)) < 0)
	{
		close(_sockfd);
		throw server_exception("Socket configuration failed. ( setsockopt() )\nShutting down server.\n");
	}
	cerr << "SO_REUSEADDR set" << endl;

	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &temporary, sizeof(int)) < 0)
	{
		close(_sockfd);
		throw server_exception("Socket configuration failed. ( setsockopt() )\nShutting down server.\n");
	}
	cerr << "SO_REUSEPORT set" << endl;

	if (bind(	_sockfd,									\
				reinterpret_cast<sockaddr*>(&_server_addr), \
				sizeof(_server_addr))						\
				< 0)
	{
		close(_sockfd);
		throw server_exception("bind() failed.\nShutting down server.\n");
	}
	cerr << "Socket successfully bound to :\n"						\
		 <<	"ip-adress: " << _server_addr.sin_addr.s_addr << endl	\
		 <<	"port :     " << ntohs(_server_addr.sin_port) << endl;

	if (listen(_sockfd, MAX_ACCEPT_QUEUE) < 0)
	{
		close(_sockfd);
		throw server_exception("listen() failed.\nShutting down server.\n");
	}
	cerr << "Socket listening" << endl;
}

server::~server()
{
	cerr	<< "server destructor for port " << _port << "\n" \
			<< "                    socket " << _sockfd << endl;
}
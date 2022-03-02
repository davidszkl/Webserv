#include "server.hpp"

server::server(const int port): _port(port)
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	cerr << "SOCKFD IS " << _sockfd << endl;
	if (_sockfd < 0)
		throw server_error("socket creation failed.\nShutting down server.\n");
	cerr << "Socket successfully created" << endl;

	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_sockfd);
		throw server_error("socket configuration failed.\nShutting down server.\n");
	}
	cerr << "O_NONBLOCK set" << endl;

	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family			= AF_INET;
	_server_addr.sin_addr.s_addr	= inet_addr("0.0.0.0");
	_server_addr.sin_port			= htons(port);

	int temporary = 0;
	setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &temporary, sizeof(int));

	if (bind(	_sockfd,									\
				reinterpret_cast<sockaddr*>(&_server_addr), \
				sizeof(_server_addr))						\
				< 0)
	{
		close(_sockfd);
		throw server_error("bind() failed.\nShutting down server.\n");
	}
	cerr << "Socket successfully bound to :\n"						\
		 <<	"ip-adress: " << _server_addr.sin_addr.s_addr << endl	\
		 <<	"port :     " << ntohs(_server_addr.sin_port) << endl;

	if (listen(_sockfd, MAX_ACCEPT_QUEUE) < 0)
	{
		close(_sockfd);
		throw server_error("listen() failed.\nShutting down server.\n");
	}
	cerr << "Socket listening" << endl;

	(void)_client_addr;
	(void)_socket_len;
	(void)_poll_rval;
}

server::~server()
{
	cout << "server destructor for port " << _port << endl;
	if (_sockfd)
		close(_sockfd);	
}

void server::test_alive() const {
	cout << "server listening on port " << _port << " alive\n";
}
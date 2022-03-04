#include "server.hpp"

server::server(const int port): _port(port)
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
		throw server_exception("socket creation failed.\nShutting down server.\n");
	cerr << "Socket " << _sockfd << " successfully created" << endl;

	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_sockfd);
		throw server_exception("socket configuration failed. ( fcntl() )\nShutting down server.\n");
	}
	cerr << "O_NONBLOCK set" << endl;

	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family			= AF_INET;
	_server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	_server_addr.sin_port			= htons(port);

	int temporary = 0;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &temporary, sizeof(int)) < 0)
	{
		close(_sockfd);
		throw server_exception("socket configuration failed. ( setsockopt() )\nShutting down server.\n");
	}	

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

void server::test_alive() const {
	cout << "server listening on port " << _port << " alive\n";
}
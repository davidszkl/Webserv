#include <errno.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

#define PORT 4269

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	FLAG_GET_ERROR,
	FLAG_SET_ERROR,
	BIND_ERROR,
	LISTEN_ERROR,
	ACCEPT_ERROR,
	POLL_ERROR
};

bool g_server_alive = true;

void sigsig(int nbr) {
	(void)nbr;
	g_server_alive = false;
}

void chat(pollfd* pollfds) {
	while (true)
	{
		char buffer[101];
		int rval = poll(pollfds, 1, 1000);
		if (rval == 0) continue;
		if (rval == -1)
		{
			perror("poll()");
			break;
		}
		if (pollfds[0].revents & POLLIN)
		{
			cerr << "receiving message:\n";
			int end = recv(pollfds[0].fd, &buffer, 100, 0);
			if (end == -1)
			{
				perror("recv()");
				break;
			}
			else if (end == 0)
			{
				cerr << "Received nothing.\n";
				break;
			}
			buffer[end] = 0;
			cout << buffer << endl;
		}
	}
	cerr << "Quitting\n";
}

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM , 0);
	if (sockfd == -1) {
		perror("socket():");
		return SOCKET_ERROR;
	}
	cerr << "Successfully created socket" << endl;

	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl()");
		close(sockfd);
		return FLAG_SET_ERROR;
	}
	cerr << "O_NONBLOCK set" << endl;
	
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(PORT);

	if (bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("bind()");
		return BIND_ERROR;
	}
	cerr << "Socket successfully bound" << endl;

	if (listen(sockfd, 10) == -1) {
		perror("listen()");
		close(sockfd);
		return LISTEN_ERROR;
	}
	cerr << "Socket " << sockfd << " listening" << endl;
	
	while (g_server_alive)
	{
		pollfd pollfds[10]; // 10 just for the example. only gonna  use one
		int rval = 0;
		pollfd pollsock;
		pollsock.fd = sockfd;
		pollsock.events = POLLIN;
		while (rval == 0)
		{
			cerr << "Waiting for connection.\r";
			if ((rval = poll(&pollsock, 1, 1000)) != 0)
				break ;
			cerr << "Waiting for connection..\r";
			if ((rval = poll(&pollsock, 1, 1000)) != 0)
				break ;
			cerr << "Waiting for connection...\r";
			if ((rval = poll(&pollsock, 1, 1000)) != 0)
				break ;
			cerr << "                         \r";
		}
		cerr << endl;
		if (rval == -1)
		{
			perror("poll()");
			close(sockfd);
			return POLL_ERROR;
		}
		sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		pollfds[0].fd = accept(sockfd, (sockaddr*)&client_addr, &len);
		pollfds[0].events = POLLIN;
		if (pollfds[0].fd == -1)
		{
			perror("accept()");
			close(sockfd);
			return ACCEPT_ERROR;
		}
		cerr << "Successfully accepted incoming request" << endl;
		chat(pollfds);
		close(pollfds[0].fd);
	}
	close(sockfd);
	return 0;
}

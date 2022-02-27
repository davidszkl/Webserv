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

#define PORT 4269
#define sockaddr_in struct sockaddr_in
#define SA struct sockaddr
#define pollfd struct pollfd

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	FLAG_GET_ERROR,
	FLAG_SET_ERROR,
	BIND_ERROR,
	LISTEN_ERROR,
	ACCEPT_ERROR
};

void chat(int connfd, pollfd* poll_var) {
	char buffer[100];
	memset(buffer, 0, 100);
	int rval = -1;
	while (rval <= 0) {
		rval = poll(poll_var, connfd, 1);
	}
	cout << "rval in chat = " << rval << endl;
	recv(connfd, &buffer, 100, 0);
	cout << buffer << endl;
}

int main()
{
	//socket()
	int sockfd = socket(AF_INET, SOCK_STREAM , 0);
	if (sockfd < 0) {
		cout << "Couldn't open socket" << endl;
		return SOCKET_ERROR;
	}
	cout << "Successfully created socket" << endl;

	//fcntl() for O_NONBLOCK
	int flags = fcntl(sockfd, F_GETFL);
	if (flags < 0) {
		cout << "Couldn't get flags" << endl;
		close(sockfd);
		return FLAG_GET_ERROR;
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		cout << "Couldn't set O_NONBLOCK" << endl;
		close(sockfd);
		return FLAG_SET_ERROR;
	}
	cout << "O_NONBLOCK set" << endl;

	//set server_addr struct
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(PORT);

	//bind socket to server_addr struct
	if (bind(sockfd, (SA*)&server_addr, sizeof(server_addr)) < 0) {
		cout << "Couldn't bind socket" << endl;
		return BIND_ERROR;
	}
	cout << "Socket successfully bound" << endl;

	//make socket listen
	if (listen(sockfd , 10) < 0) {
		cout << "Couldn't set listen on socket " << sockfd << endl;
		close(sockfd);
		return LISTEN_ERROR;
	}
	cout << "Socket " << sockfd << " listening" << endl;

	//poll incoming requests for an accept()
	sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int connfd = 0;
	pollfd server_fd;
	server_fd.fd = sockfd;
	server_fd.events = POLLIN; 
	int rval = -1;
	while (1)
	{
		while (rval <= 0) {
			rval = poll(&server_fd, 1, 1);
		}
		cout << "rval " << rval << endl;
		connfd = accept(sockfd, (SA*)&client_addr, &len);
		if (connfd < 0)
		{
			cout << "Coudln't accept incoming request" << endl;
			cout << "connfd = " << connfd << "\nerrno = " << errno << endl;
			close(connfd);
			return ACCEPT_ERROR;
		}
		cout << "Successfully accepted incoming request" << endl;
		chat(connfd, &server_fd);
		close(connfd);
	}
	return 0;
}
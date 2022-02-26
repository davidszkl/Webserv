#include <iostream>
#include <cstring>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6969
#define sockaddr_in struct sockaddr_in
#define SA struct sockaddr

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	BIND_ERROR,
	LISTEN_ERROR,
	ACCEPT_ERROR
};

void chat(int connfd) {
	char buffer[100];
	while (1) {
		read(connfd, &buffer, 100);
		if (!strcmp(buffer, "stop")) {
			cout << "stopping server" << endl;
			break;
		}
		cout << buffer << endl;
	}
}

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM , 0);
	if (sockfd < 0) {
		cout << "Couldn't open socket" << endl;
		return SOCKET_ERROR;
	}
	cout << "Successfully created socket" << endl;

	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	printf("addr = %u\n", server_addr.sin_addr.s_addr);
	server_addr.sin_port		= htons(PORT);
	if (bind(sockfd, (SA*)&server_addr, sizeof(server_addr)) < 0) {
		cout << "Couldn't bind socket" << endl;
		return BIND_ERROR;
	}
	cout << "Socket successfully bound" << endl;

	if (listen(sockfd , 10) < 0) {
		cout << "Couldn't set listen on socket " << sockfd << endl;
		return LISTEN_ERROR;
	}
	cout << "Socket " << sockfd << " listening" << endl;

	sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int connfd = accept(sockfd, (SA*)&client_addr, &len);
	if (connfd < 0) {
		cout << "Coudln't accept incoming request" << endl;
		return ACCEPT_ERROR;
	}
	cout << "Successfully accepted incoming request" << endl;

	chat(connfd);

	close(sockfd);
	return 0;
}
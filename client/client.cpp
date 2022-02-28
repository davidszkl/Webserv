#include <iostream>
#include <fstream>
#include <cstring>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define PORT 4269
#define sockaddr_in struct sockaddr_in
#define SA struct sockaddr

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	CONNECT_ERROR
};

void chat(int sockfd) {
	std::string buffer;
	getline(cin, buffer);
	send(sockfd, buffer.c_str(), buffer.length(), 0);
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
	server_addr.sin_port		= htons(PORT);


	// if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
	// 	cout << "Couldn't set O_NONBLOCK" << endl;
	// 	close(sockfd);
	// }
	// cout << "O_NONBLOCK set" << endl;

	#define pollfd struct pollfd
	// pollfd server_fd;
	// server_fd.fd = sockfd;
	// server_fd.events = POLLOUT; 
	// int rval = -1;
	// cout << "coucou\n";
	// 	while (rval <= 0) {
	// 		rval = poll(&server_fd, 1, 1);
	// 	}
	// cout << "rval " << rval << endl;


	if (connect(sockfd, (SA*)&server_addr, sizeof(server_addr)) < 0) {
		cout << "Coudln't connect" << endl;
		cout << "errno = " << errno << endl;
		return CONNECT_ERROR;
	}
	cout << "Successfully connected" << endl;

	chat(sockfd);
	close(sockfd);
	return 0;
}
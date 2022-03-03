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

#define PORT 42690

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	CONNECT_ERROR,
	CLOSE_ERROR
};

void chat(int sockfd) {
	while (1)
	{
		std::string buffer;
		cerr << "Enter message to send -- empty to quit: ";
		getline(cin, buffer);
		if (buffer.size() == 0)
			break;
		cerr << "Sending message..." << endl;
		ssize_t rval = send(sockfd, buffer.c_str(), buffer.length(), 0);
		if (rval == -1)
			perror("send()");
		else if ((size_t)rval != buffer.length())
			cerr << "send() sent " << rval << " bytes instead of "
				<< buffer.length() << " bytes" << endl;
		else
			cerr << "Message sent successfully" << endl;
	}
	cerr << "Quitting..." << endl;
}

int main()
{
	cerr << "Creating socket..." << endl;
	int sockfd = socket(AF_INET, SOCK_STREAM , 0);
	if (sockfd < 0) {
		perror("socket()");
		return SOCKET_ERROR;
	}
	cerr << "Successfully created socket" << endl;
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(PORT);
	cerr << "Connecting..." << endl;
	if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
		perror("connect()");
		close(sockfd);
		return CONNECT_ERROR;
	}
	cerr << "Successfully connected" << endl;
	chat(sockfd);
	if (-1 == close(sockfd))
	{
		perror("close()");
		return CLOSE_ERROR;
	}
	return 0;
}

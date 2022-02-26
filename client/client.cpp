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


#define PORT 6969
#define sockaddr_in struct sockaddr_in
#define SA struct sockaddr

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	CONNECT_ERROR
};

void chat(int sockfd) {
	std::string buffer;
	while (1) {
		getline(cin, buffer);
		if (buffer == "stop") {
			cout << "ending connection" << endl;
			break ;
		}
		send(sockfd, buffer.c_str(), buffer.length(), 0);
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
	cout << "addr = " << server_addr.sin_addr.s_addr << endl;
	server_addr.sin_port		= htons(PORT);

	if (connect(sockfd, (SA*)&server_addr, sizeof(server_addr)) < 0) {
		cout << "Coudln't connect" << endl;
		return CONNECT_ERROR;
	}
	cout << "Successfully connected" << endl;

	chat(sockfd);
	close(sockfd);
	return 0;
}
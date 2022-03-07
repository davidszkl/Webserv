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

#define PORT 42692

using namespace std;

enum EXIT_ERRORS  {
	SOCKET_ERROR = 1,
	CONNECT_ERROR,
	CLOSE_ERROR
};
#include <sstream>

std::string slurp_file(std::string file) {
	std::ifstream stream(file);
	std::stringstream buffer;
	buffer << stream.rdbuf();
	std::string file_content("GET /index.html HTTP/1.1\r\nline1\r\nline2\r\n\r\n");
	file_content += buffer.str();
	return file_content;
}

void send_html(int sockfd) {
	std::string tmp(slurp_file("../index.html"));
	send(sockfd, tmp.c_str() , tmp.size(), 0);
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
	send_html(sockfd);
	if (-1 == close(sockfd))
	{
		perror("close()");
		return CLOSE_ERROR;
	}
	return 0;
}

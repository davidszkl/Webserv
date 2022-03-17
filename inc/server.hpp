#pragma once

#include "utilities.hpp"
#include "config.hpp"

#define MAX_ACCEPT_QUEUE 128	//size of the queue for the socket

class server
{
public:

	server(config conf);
	~server();

	class server_exception : public std::runtime_error
	{
	public:
		server_exception(const char* what) : runtime_error(what) {}
	};

	void test_alive() const;
private:

	friend class webserver;
	
	int							_sockfd;
	unsigned short 				_port;
	size_t 						_max_body;
	string 						_server_name;
	sockaddr_in					_server_addr;
	map<int, string> 			_error_pages;
	vector<config::location>	_location_blocks;
		//string path;
		//string root;
		//string index;
		//string redirect;
		//string upload_dir;
		//vector<string> allowed_methods;

};

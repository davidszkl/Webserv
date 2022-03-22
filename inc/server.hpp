#pragma once

#include "utilities.hpp"
#include "config.hpp"

#define MAX_ACCEPT_QUEUE 128	//size of the queue for the socket

class server
{
public:

	server(vector<config> vector_config);
	~server();

	class server_exception : public std::runtime_error
	{
	public:
		server_exception(const char* what) : runtime_error(what) {}
	};

	friend class webserver;

private:

	vector<config>	_configs;
	string			_full_url;
	sockaddr_in		_server_addr;
	unsigned short	_port;
	int				_sockfd;

	// class location 
	// 	friend class server;
	// 	friend class webserver;
	// 	std::string _path;			// contains location path last '/' will be removed
	// 	std::string _root;			// empty if none
	// 	std::string _index;			//empty if none
	// 	std::string _redirect;		// empty if no redirect
	// 	std::string _upload_dir;	//empty if upload not permitted
	// 	std::vector<std::string> _allowed_methods;
	// 	bool		_autoindex;
	// 	location() {_autoindex = false;};
	
	// int							_sockfd;
	// unsigned short 				_port;
	// size_t 						_max_body;			//post
	// string 						_server_name;		//ost:
	// map<int, string> 			_error_pages;
	// vector<config::location>	_location_blocks;
		//string path;
		//string root;
		//string index;
		//string redirect;
		//string upload_dir;
		//vector<string> allowed_methods;

	typedef map<int, string>::iterator map_it;
};

#pragma once
#include "server.hpp"

class webserver
{
public:

	webserver()  {};
	~webserver() {};

	void add_server(int port);

private:
	std::vector<server> _servers;
};

#include "webserver.hpp"

void webserver::add_server(int port)
{
	try {
		_servers.push_back(server(port));
	}
	catch (server::server_error& e) {
		cerr << e.what() << endl;
	}
}
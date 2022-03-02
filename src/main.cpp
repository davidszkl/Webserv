#include "webserver.hpp"
int main()
{
	std::vector<int> ports;
	ports.push_back(42690);
	ports.push_back(42691);
	ports.push_back(42692);				//config class supplied by mlefevre

	webserver main_server;				//class that holds all the virtual servers
	for (size_t n = 0; n < ports.size(); n++)
	{
		main_server.add_server(ports[n]);
	}

	// for (size_t n = 0; n < 3 ; n++)
	// {
	// 	cout << main_server._pollsock[n].fd;
	// }
	main_server.listen_all();

	return 0;
}
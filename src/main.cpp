#include "webserver.hpp"
#ifndef TEST
int main()
{
	std::vector<int> ports;
	ports.push_back(42690);
	ports.push_back(42691);
	ports.push_back(42692);				//config class supplied by mlefevre

	webserver main_server(ports.size());				//class that holds all the virtual servers
	for (size_t n = 0; n < ports.size(); n++)
	{
		try {
			main_server.add_server(ports[n]);
		}
		catch (webserver::webserver_exception &e) {
			cerr << e.what() << endl; 
		}
	}
	try {
		main_server.listen_all();
	}
	catch (webserver::webserver_exception &e) {
			cerr << e.what() << endl; 
	}
	cerr << "out of listen_all" << endl;
	return 0;
}
#endif
#ifdef TEST
int main()
{
	int fd1 = 0, fd2 = 0, fd3 = 0;
	fd1 = socket(AF_INET, SOCK_STREAM, 0);
	cout << "fd1 " << fd1 << endl;
	cout << "fd2 " << fd2 << endl;
	cout << "fd3 " << fd3 << endl;
	fd2 = socket(AF_INET, SOCK_STREAM, 0);
	cout << "fd1 " << fd1 << endl;
	cout << "fd2 " << fd2 << endl;
	cout << "fd3 " << fd3 << endl;
	fd3 = socket(AF_INET, SOCK_STREAM, 0);
	cout << "fd1 " << fd1 << endl;
	cout << "fd2 " << fd2 << endl;
	cout << "fd3 " << fd3 << endl;
}
#endif
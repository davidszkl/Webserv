#include "webserver.hpp"
enum error_codes {
	VECTOR_INIT_ERROR = 1,
	RUNTIME_ERROR
};

int main()
{
	std::vector<int> ports;
	ports.push_back(42690);
	ports.push_back(42691);
	ports.push_back(42692);				//config class supplied by mlefevre

	webserver main_server(ports.size());				//class that holds all the virtual servers

	try {
		for (size_t n = 0; n < ports.size(); n++)
			main_server.add_server(ports[n]);
		main_server.listen_all();
	}
	catch (server::server_exception &e) {
		cerr << e.what() << endl;
		return VECTOR_INIT_ERROR;
	}
	catch (webserver::webserver_exception &e) {
		cerr << e.what() << endl;
		return RUNTIME_ERROR;
	}
	return 0;
}
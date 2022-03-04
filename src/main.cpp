#include "webserver.hpp"
enum error_codes {
	VECTOR_INIT_ERROR = 1,
	RUNTIME_ERROR,
	FATAL_ERROR
};

int main()
{
	std::vector<int> config;
	config.push_back(42690);
	config.push_back(42691);
	config.push_back(42692);					//config class supplied by mlefevre

	try {
		webserver main_server(config);			//all servers initialized in webserver constructor
		main_server.listen_all();				//main function listening on all servers
	}
	catch (server::server_exception &e) {		//one of the servers couldn't be initialized
		cerr << e.what() << endl;
		return VECTOR_INIT_ERROR;
	}
	catch (webserver::webserver_exception &e) {	//runtime error: accept(), recv(), etc..
		cerr << e.what() << endl;
		return RUNTIME_ERROR;
	}
	catch (...) {								//other errors like bad_alloc
		cerr << "Fatal error.\nShutting down server..." << endl;
		return FATAL_ERROR;
	}
	return 0;
}
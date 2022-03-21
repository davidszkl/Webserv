#pragma once

#include <vector>		//webserver, config
#include <map>			//config, server

#include <string>		//string_utils, config, cgi
#include <cstring>		//server
#include <cstdlib>		//webserver
#include <utility>		//config

#include <fstream>		//webserver
#include <sstream>		//string_utils, server
#include <iostream>		//server, debug, config

#include <fcntl.h>		//server
#include <unistd.h>		//server

#include <arpa/inet.h>	//server
#include <netinet/in.h>	//server
#include <poll.h>		//server

#include <stdexcept>	//server

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;
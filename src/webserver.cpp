#include "webserver.hpp"
#include "debug.hpp"

bool _server_alive = true;

//TO_ERASE
void server_shutdown(int signbr) {
	(void)signbr;
	_server_alive = false;
}
//TO_ERASE

webserver::webserver(vector<config> config_vector):	_response_code(404), _socklen(sizeof(_client_addr))
{
	signal(SIGINT, &server_shutdown);
	_server_alive = true;
	memset(&_pollfd, 0, sizeof(_pollfd));
	_pollfd.fd	= -1;
	try {
		map<unsigned short, vector<config> > ports;
		for(size_t n = 0; n < config_vector.size(); n++)
			ports[config_vector[n].port].push_back(config_vector[n]);
		for(map_it n = ports.begin(); n != ports.end(); n++)
			_servers.push_back(server(n->second));
	}
	catch (...) {
		throw ;
	}
	for (size_t n = 0; n < _servers.size(); n++)
	{
		cout << "PORT " << ntohs(_servers[n]._port) << endl;
		cout << "SIZE " << _servers[n]._configs.size() << endl;
		for (size_t j = 0; j < _servers[n]._configs.size(); j++)
			cout << "NAME [" << n << "][" << j << "]" <<  _servers[n]._configs[j].server_name << endl;
	}
}

webserver::~webserver() {
	logn("webserv destructor");
	for (size_t n = 0; n < _servers.size(); n++) {					//~webserv() handles closing of server sockets because
		if (_servers[n]._sockfd)
		{
			cerr << "CLOSING FD " << _servers[n]._sockfd << endl;	//vector calls server destructor all the time
			close(_servers[n]._sockfd);
		}
	}
}

void webserver::init_pollsock()
{
	pollfd listener;
	memset(&listener, 0, sizeof(listener));
	_pollsock.push_back(listener);
	for (size_t n = 0; n < _servers.size(); n++)		//initialize vector of pollfd that contains all server-sockets
	{
		pollfd tmp;
		memset(&tmp, 0, sizeof(tmp));
		tmp.fd		= _servers[n]._sockfd;
		tmp.events	= POLLIN | POLLOUT;
		_pollsock.push_back(tmp);
	}
}

int webserver::get_fd_ready() const {					//get the first open fd out of poll()
	for (size_t n = 0; n < _pollsock.size(); n++)
		if (_pollsock[n].revents & POLLIN)
			return _pollsock[n].fd;
	return -1;
}

void webserver::listen_all()
{
	init_pollsock();
	int accept_fd = -1;
	while (_server_alive)
	{
		int rval = 0;
		while (true)
		{
			cerr << "Waiting for connection.\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)))
				break ;
			cerr << "Waiting for connection..\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)))
				break ;
			cerr << "Waiting for connection...\r";
			if ((rval = poll(&_pollsock[0], _pollsock.size(), 1000)))
				break ;
			cerr << "                         \r";
		}
		cerr << endl;
		if (rval < 0)
			throw webserver_exception("Poll failed on an fd");

		if (accept_fd < 0)
		{
			accept_fd = get_fd_ready();
			cerr << "accept_fd " << accept_fd << endl;
			if ((_pollsock[0].fd = accept(accept_fd,								\
										reinterpret_cast<sockaddr*>(&_client_addr),	\
										&_socklen))									\
										< 0)
				throw webserver_exception("Accept failed");
			_pollsock[0].events = POLLIN | POLLOUT;
			cerr << "Connection on fd " << accept_fd << " accepted" << endl \
				 << "Connection fd is " << _pollsock[0].fd << endl;
			continue ;
		}

		if (!(_pollsock[0].revents & POLLIN))
			continue ;
		int read_rval = read_msg(_pollsock[0].fd);
		if (read_rval == -1) {
			cerr << "Fatal problem occured during connection with " << accept_fd << endl;
			cerr << "ERRNO " << errno << endl;
			throw webserver_exception("Poll fatal error");
		}
		try {
			int id = get_server_id(accept_fd);
			request_handler(_pollsock[0], _servers[id]);
		}
		catch (webserver_exception & e) {
			cerr << e.what() << endl;
		}
		close(_pollsock[0].fd);
		_pollsock[0].fd		= 0;
		_pollsock[0].events = 0;
		accept_fd = -1;
		clear_errors();
	}
}

int webserver::read_msg(int fd) {;
	char buffer[100] = {0};
	int end = 0;
	clear_request();
	cerr << "Receiving message:\n";
	while(!find_crlf(string(buffer)))
	{
		end = recv(fd, &buffer, 100, 0);
		if (end < 0)
			return -1;
		buffer[end] = '\0';
		_http_request._full_request += buffer;
	}
	if (is_post(_http_request._full_request))
	{
		_content_length = std::atoi(get_header_info(_http_request._full_request, "Content-Length").c_str());
		while(!find_crlf(string(buffer)))
		{
			end = recv(fd, &buffer, 100, 0);
			if (end < 0)
				return -1;
			buffer[end] = '\0';
			_http_request._full_request += buffer;
		}
	}
	logn("received all of the message");
	return 0;
}

bool find_crlf(string str) {
	if (str.find("\r\n\r\n") != string::npos)
		return true;
	return false;
}

bool is_post(string str) {
	if (str.find("POST") != string::npos)
		return true;
	return false;
}

void webserver::request_handler(const pollfd & fd, server & server) {
	init_request(server);
	logn("request=================");
	logn(_http_request._full_request);
	logn("request=================");
	logn("uri: " + _http_request._full_request);
	if (!_http_request._method.size()	||
		!_http_request._uri.size()		||
		!_http_request._version.size())
	{
		_response_code = BAD_REQUEST;
		send_response(fd, "", false);
	}
	else if (_http_request._uri.size() > 1024)
	{
		_response_code = REQUEST_URI_TOO_LONG;
		send_response(fd, "", false);
	}
	else if (_http_request._version != "HTTP/1.1")
	{
		_response_code = HTTP_VERSION_NOT_SUPPORTED;
		send_response(fd, "", false);
	}
	else if (_http_request._method	== "GET")
		handle_GET(fd, server);
	else if (_http_request._method	== "POST")
		handle_POST(fd);
	else if (_http_request._method	== "DELETE")
		handle_DELETE(fd, server);
	else {
		_response_code = NOT_IMPLEMENTED;
		send_response(fd, server._configs[_config_index].error_pages[NOT_IMPLEMENTED], true);
	}
}

int	webserver::handle_GET(const pollfd &fd, server & server) {
	bool body					= true;
	std::string& response_file	= _http_request._path;
	const config::location & current_block = server._configs[_config_index].location_blocks[_location_index];
	if (std::find(current_block.allowed_methods.begin(), current_block.allowed_methods.end(),
	 	"GET")
		== current_block.allowed_methods.end())
	{
		_response_code = METHOD_NOT_ALLOWED;
		response_file = server._configs[_config_index].error_pages[METHOD_NOT_ALLOWED];
	}
	if (_http_request._path[_http_request._path.size() - 1] == '/')
		_http_request._path += current_block.index;
	logn("requestpath: " + _http_request._path);
    if(current_block.autoindex)
    {
       response_file =  autoindex(_http_request._path);
    }
	else if (!file_exists(_http_request._path)) {
		_response_code = NOT_FOUND;
		response_file = server._configs[_config_index].error_pages[NOT_FOUND];
	}
	else if (_http_request._path.find("server_files") == string::npos) {
		_response_code = FORBIDDEN;
		response_file = server._configs[_config_index].error_pages[FORBIDDEN];
	}
	else
		_response_code = OK;
	logn("RESPONSE_FILE\n" + response_file);
	send_response(fd, response_file, body);
	return 0;
}

int	webserver::handle_POST(const pollfd &fd) {
    bool body					= true;
    std::string& response_file	= _http_request._path;
    const config::location & current_block = server._configs[_config_index].location_blocks[_location_index];

    if (std::find(current_block.allowed_methods.begin(), current_block.allowed_methods.end(),"POST")== current_block.allowed_methods.end())
    {
        _response_code = METHOD_NOT_ALLOWED;
        response_file = server._configs[_config_index].error_pages[METHOD_NOT_ALLOWED];
    }
    if (_http_request._path[_http_request._path.size() - 1] == '/')
        _http_request._path += current_block.index;
    logn("requestpath: " + _http_request._path);
    if (!file_exists(_http_request._path)) {
        _response_code = NOT_FOUND;
        response_file = server._configs[_config_index].error_pages[NOT_FOUND];
    }
    else if (_http_request._path.find("server_files") == string::npos) {
        _response_code = FORBIDDEN;
        response_file = server._configs[_config_index].error_pages[FORBIDDEN];
    }
    else
        _response_code = OK;
    logn("RESPONSE_FILE\n" + response_file);
    send_response(fd, response_file, body);
    return 0;
}

int	webserver::handle_DELETE(const pollfd &fd, server& server) {
	bool body					= true;
	std::string& response_file	= _http_request._path;
	const config::location & current_block = server._configs[_config_index].location_blocks[_location_index];

	if (std::find(current_block.allowed_methods.begin(), current_block.allowed_methods.end(),
	 	"DELETE") == current_block.allowed_methods.end())
	{
		_response_code = METHOD_NOT_ALLOWED;
		response_file = server._configs[_config_index].error_pages[METHOD_NOT_ALLOWED];
	}
	if (!file_exists(_http_request._path)) {
		_response_code = NOT_FOUND;
		response_file = server._configs[_config_index].error_pages[NOT_FOUND];
	}
	else if (_http_request._path.find("server_files") == std::string::npos) {
		_response_code = FORBIDDEN;
		response_file = server._configs[_config_index].error_pages[FORBIDDEN];
	}
	else
	{
		if (is_deletable(server, response_file))
		{
			_response_code = NO_CONTENT;
			//remove ressource;
			body = false;
		}
		else
		{
			_response_code = FORBIDDEN;
			response_file = server._configs[_config_index].error_pages[FORBIDDEN];
		}
	}
	send_response(fd, response_file, body);
	return 0;
}

void webserver::send_response(const pollfd &fd, string filename, bool body) {
	string http_response;
	http_response += "HTTP/1.1 ";
	http_response += i_to_str(_response_code);
	http_response += get_code_description(_response_code);
	if (body) {
		http_response += "\r\n\r\n";
		http_response += slurp_file(filename);
	}
	send(fd.fd, http_response.c_str(), http_response.size(), 0);
}

void webserver::init_request(const server & server) {
	int temp_pos = _http_request._full_request.find("\r\n\r\n");
	if (temp_pos > 0)
		_http_request._header = _http_request._full_request.substr(0, temp_pos);
	_http_request._body = _http_request._full_request.substr(temp_pos + 4, _http_request._full_request.size());

	size_t cursor = 0;
	while(1)
	{
		string str_from(&_http_request._header[cursor]);
		string line = read_header_line(&_http_request._header[cursor]);
		cursor += line.size();
		_http_request._header_lines.push_back(line);
		if (line == "\r\n" || !line.size())
			break ;
	}
	std::stringstream ss(_http_request._header_lines[0]);
	ss >> _http_request._method;
	ss >> _http_request._uri;
	ss >> _http_request._version;
	_config_index = get_config_index(server._port, server._configs, _http_request._header_lines);
	if (_config_index == -1)
	{
		logn("config index is -1. setting path to " + _http_request._uri);
		_http_request._path = _http_request._uri;
		return;
	}
	_location_index = get_location_index(_http_request._uri, server._configs[_config_index]);
	const std::size_t l = server._configs[_config_index].location_blocks[_location_index].path.length();
	_root = server._configs[_config_index].location_blocks[_location_index].root;
	_http_request._path = _root + _http_request._uri.substr(l, _http_request._uri.length() - l);
	logn("new request_path: " + _http_request._path);
	logn("used root: " + _root);
}

void webserver::clear_errors() {						//clear servers that got shutdown for some reason
	for (size_t n = 1; n < _pollsock.size(); n++)
	{
		if (_pollsock[n].revents & POLLERR)
		{
			cerr << "killing server " << n << endl;
			close(_pollsock[n].fd);
			_servers.erase(_servers.begin() + n);
			_pollsock.erase(_pollsock.begin() + n);
		}
	}
}

inline bool file_exists (const string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

string webserver::get_code_description(int code) const {
	switch (code) {
		case OK :
			return " OK";
		case CREATED :
			return " CREATED";
		case ACCEPTED :
			return " ACCEPTED";
		case NO_CONTENT :
			return " NO_CONTENT";
		case RESET_CONTENT :
			return " RESET_CONTENT";
		case PARTIAL_CONTENT :
			return " PARTIAL_CONTENT";
		case MULTIPLE_CHOICES :
			return " MULTIPLE_CHOICES";
		case MOVED_PERMANENTLY :
			return " MOVED_PERMANENTLY";
		case FOUND :
			return " FOUND";
		case SEE_OTHER :
			return " SEE_OTHER";
		case NOT_MODIFIED :
			return " NOT_MODIFIED";
		case USE_PROXY :
			return " USE_PROXY";
		case TEMPORARY_REDIRECT :
			return " TEMPORARY_REDIRECT";
		case BAD_REQUEST :
			return " BAD_REQUEST";
		case UNAUTHORIZED :
			return " UNAUTHORIZED";
		case FORBIDDEN :
			return " FORBIDDEN";
		case NOT_FOUND :
			return " NOT_FOUND";
		case METHOD_NOT_ALLOWED :
			return " METHOD_NOT_ALLOWED";
		case NOT_ACCEPTABLE :
			return " NOT_ACCEPTABLE";
		case PROXY_AUTHENTICATION_REQUIRED :
			return " PROXY_AUTHENTICATION_REQUIRED";
		case REQUEST_TIMEOUT :
			return " REQUEST_TIMEOUT";
		case CONFLICT :
			return " CONFLICT";
		case GONE :
			return " GONE";
		case LENGTH_REQUIRED :
			return " LENGTH_REQUIRED";
		case PRECONDITION_FAILED :
			return " PRECONDITION_FAILED";
		case REQUEST_ENTITY_TOO_LARGE :
			return " REQUEST_ENTITY_TOO_LARGE";
		case REQUEST_URI_TOO_LONG :
			return " REQUEST_URI_TOO_LONG";
		case UNSUPPORTED_MEDIA_TYPE :
			return " UNSUPPORTED_MEDIA_TYPE";
		case REQUESTED_RANGE_NOT_SATISFIABLE :
			return " REQUESTED_RANGE_NOT_SATISFIABLE";
		case EXPECTATION_FAILED :
			return " EXPECTATION_FAILED";
		case INTERNAL_SERVER_ERROR :
			return " INTERNAL_SERVER_ERROR";
		case NOT_IMPLEMENTED :
			return " NOT_IMPLEMENTED";
		case BAD_GATEWAY :
			return " BAD_GATEWAY";
		case SERVICE_UNAVAILABLE :
			return " SERVICE_UNAVAILABLE";
		case GATEWAY_TIMEOUT :
			return " GATEWAY_TIMEOUT";
		default :
			return " ";
	}
}

string slurp_file(string file) {
	std::ifstream stream(file.c_str());
	std::stringstream buffer;
	buffer << stream.rdbuf();
	string file_content(buffer.str());
	if (!(file == "server_files/favicon.ico"))
		logn("WEBPAGE\n" + file_content + "WEBPAGE\n");
	return file_content;
}

string i_to_str(int nbr) {
	std::stringstream ss;
    string s;
    ss << nbr;
    s = ss.str();
    return s;
}

string my_get_line(string from ) {
	string to;
	for (size_t n = 0; from[n] && from[n] != '\n'; n++)
		to += from[n];
	return to;
}

string read_header_line(string from) {
	string str;
	for (size_t n = 0; from[n]; n++)
	{
		str += from[n];
		if (str[n] == '\n' && str.size() > 2 && str[n - 1] == '\r')
			break;
	}
	return str;
}

void poll_result(const pollfd & fd){
	cerr << "events  : "												\
		 << (fd.events & POLLIN 	? " POLLIN |"	: "        |")		\
		 << (fd.events & POLLHUP	? " POLLHUP |"	: "         |")		\
		 << (fd.events & POLLERR	? " POLLERR |"	: "         |")		\
		 << (fd.events & POLLPRI	? " POLLPRI |"	: "         |")		\
		 << (fd.events & POLLOUT	? " POLLOUT |"	: "         |")		\
		 << (fd.events & POLLNVAL	? " POLLNVAL |"	: "          |")	<< endl;
	cerr << "revents : " 												\
		 << (fd.revents & POLLIN 	? " POLLIN |"	: "        |")		\
		 << (fd.revents & POLLHUP	? " POLLHUP |"	: "         |")		\
		 << (fd.revents & POLLERR	? " POLLERR |"	: "         |")		\
		 << (fd.revents & POLLPRI	? " POLLPRI |"	: "         |")		\
		 << (fd.revents & POLLOUT	? " POLLOUT |"	: "         |")		\
		 << (fd.revents & POLLNVAL	? " POLLNVAL |"	: "          |")	<< endl;
}

void	webserver::clear_request() {
	_http_request._full_request.clear();
	_http_request._header.clear();
	_http_request._body.clear();
	_http_request._method.clear();
	_http_request._uri.clear();
	_http_request._version.clear();
	_http_request._header_lines.clear();
	_http_request._path.clear();
}

int webserver::get_server_id(int fd_tofind) const {
	for (size_t n = 1; n < _pollsock.size(); n++)
		if (_pollsock[n].fd == fd_tofind)
			return n - 1;
	return -1;
}

bool	webserver::is_deletable(server & server, const std::string& filename) const {
	for (server::map_it it = server._configs[0].error_pages.begin(); it != server._configs[0].error_pages.end(); it++)
		if (filename == it->second)
			return false;
	if (filename == "favicon.ico")		
		return false;
	return true;
}

int webserver::get_config_index(unsigned short _port,
    const vector<config>& _configs,
    const vector<string>& header_lines)
{
	_port = ntohs(_port);
    string host;
    for (std::size_t i = 0; i < header_lines.size(); i++)
    {
        if (header_lines[i].compare(0, 6, "Host: ") == 0)
        {
            host = header_lines[i].substr(6, string::npos);
            break;
        }
    }
    for (std::size_t i = 0; i < _configs.size(); i++)
    {
        if (_configs[i].port != _port) continue;
        if (host != "" && _configs[i].server_name != host) continue;
        return i;
    }
    for (std::size_t i = 0; i < _configs.size(); i++)
    {
        if (_configs[i].port == _port)
        	return i;
    }
	logn("get_config_index returned -1. Host=" + host + " port=" + i_to_str(_port));
    return -1;
}

int webserver::get_location_index(const string& uri, const config conf)
{
    vector<int> results;
    for (std::size_t i = 0; i < conf.location_blocks.size(); i++)
    {
        const int r = conf.location_blocks[i].match_url(uri);
        results.push_back(r);
    }
    int n = 0;
    for (std::size_t i = 0; i < results.size(); i++)
        if (results[i] > results[n])
            n = i;
   return n; 
}
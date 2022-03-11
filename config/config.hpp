#pragma once

#include <string>
#include <utility>
#include <vector>
#include <iostream>

/*
   config of a server instance.
   Instances of this class will be stored in a vector.
   thus a config file containing two server blocks will generate a vector of two config objects
   Please not that this class DOES NOT arrange anything in network byte order!
 */
class config
{
	public:
		class location {
			public:
				std::string path; // contains location path last '/' will be removed
				std::string root; // empty if none
				std::string index; //empty if none
				bool autoindex;
				std::vector<std::string> allowed_methods;
				std::string redirect; // empty if no redirect
				std::string upload_dir; //empty if upload not permitted

				/* return 0 if path does not match location. Otherwise number of '/' in this->path
				 url_path should be something like /ok/foo/bar */
				unsigned match_url(const std::string& url_path);
				location();
		};
		std::string server_name;
		unsigned short port;
		std::size_t max_body;
		std::pair<int, std::string> error_pages[5]; // error number, path/to/file
		std::vector<location> location_blocks;

		config();
};

std::ostream& operator <<(std::ostream& o, const config::location& c);
std::ostream& operator <<(std::ostream& o, const config& c);



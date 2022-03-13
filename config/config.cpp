#include "config.hpp"
#include "debug.hpp"
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <iostream>

std::ostream& operator <<(std::ostream& o, const config::location& c)
{
	o << "---location " + c.path + ":---\n";
	o << '>' << "root: " << c.root << '\n';
	o << '>' << "index: " << c.index << '\n';
	o << '>' << "autoindex: " << c.autoindex << '\n';
	o << '>' << "allowed_methods:";
	for(std::size_t i = 0; i < c.allowed_methods.size(); i++)
		o << ' ' << i << ": " << c.allowed_methods[i];
	o << '\n';
	o << '>' << "redirect: " << c.redirect << '\n';
	o << '>' << "upload_dir: " << c.upload_dir << '\n';
	return o;
}

std::ostream& operator <<(std::ostream& o, const config& c)
{
	o << "======config of " + c.server_name + ":" << c.port << "======\n";
	o << "max_body: " << c.max_body << '\n';
	o << "error_pages:\n";
	for (int i = 0; i < 5; i++)
		o << c.error_pages[i].first << ": " << c.error_pages[i].second << '\n';
	o << "location_blocks:\n";
	for (std::size_t i = 0; i < c.location_blocks.size(); i++)
		o << c.location_blocks[i];
	return o;
}

unsigned config::location::match_url(const std::string& url_path) {
	if (url_path.length() < path.length()) return 0;
	if (url_path.compare(0, path.length(), path) != 0) return 0;
	return std::count(path.begin(), path.end(), '/');
}

config::config()
: port(0), max_body(500)
{
	logn("WARNING: error_pages are not correctly initialized (TODO)");
	const std::string default_error_path = "./server_files/";
	for(int i = 0; i < 5; i++)
	{
		error_pages[i].first = 42;
		error_pages[i].second = default_error_path + "42.html";
	}

}

config::location::location()
: autoindex(false) {}
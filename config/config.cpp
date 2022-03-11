#include "config.hpp"

unsigned config::location::match_url(const std::string& url_path) {
	if (url_path.length() < path.length()) return 0;
	if (url_path.compare(0, path.length(), path) != 0) return 0;
	return std::count(path.begin(), path.end(), '/');
}


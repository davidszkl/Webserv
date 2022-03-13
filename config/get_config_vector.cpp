#include "config.hpp"
#include "to_string.hpp"
#include "debug.hpp"
#include <vector>
#include <string>
#include <algorithm>

//str is substring containing server body without the '{' and '}'
static config init_server_conf(const std::string& str, std::size_t line_num)
{
	(void)str;
	(void)line_num;
	return config();
}

//i is index of beginning '{'
//returns index of ending '}'
static std::size_t get_end_block(const std::string& str, std::size_t i)
{
	i++;
	int depth = 0;
	while (depth >= 0)
	{
		if (str[i] == '{') depth++;
		if (str[i] == '}') depth--;
		i++;
	}
	return i - 1;
}

std::vector<config> get_config_vector(const std::string& str, std::size_t line_num)
{
	std::vector<config> r;
	std::size_t i = 0;
	i = std::string(&str[i]).find("server");
	while (i < str.length())
	{
		while (str[i] != '{') i++;
		const std::size_t e = get_end_block(str, i);
		r.push_back(init_server_conf(str.substr(i + 1, e - i - 1), line_num + std::count(str.begin(), str.begin() + i, '\n')));
		i = e + 1;
		if (i == str.length()) break;
		i += std::string(&str[i]).find("server");
	}
	return r;
}

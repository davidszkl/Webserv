#include "config.hpp"
#include "to_string.hpp"
#include "debug.hpp"
#include <vector>
#include <string>
#include <algorithm>

//eturn statement (does not clean any whitespaces) without trailing ';'
//str[i] wil be == to return_value[0]
static std::string get_statement(const std::string& str, std::size_t i)
{
	std::size_t j = std::string(&str[i]).find(";");	
	return str.substr(i, j);
}

//str is substring containing server body without the '{' and '}'
static config init_server_conf(const std::string& str, std::size_t line_num)
{
	config c;
	std::size_t i = 0;
	while (i < str.length())
	{
		std::string statement = get_statement(str, i);
		std::vector<std::string> statement_split = split_statement(statement);
		if (!is_valid_statement(split_statement))
			throw std::runtime_error(to_string(line_num + std::count(str, '\n')) + ": invalid statement in server block");
		set_statement(c, split_statement);
		i += statement.length() + 1;
	}
	return c;
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
		std::size_t i2 = std::string(&str[i]).find("server");
		if (i2 == std::string::npos) break;
		i += i2;
	}
	return r;
}

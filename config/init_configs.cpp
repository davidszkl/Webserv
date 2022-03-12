#include "config.hpp"
#include <fstream>
#include <exception>
#include <string>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include "to_string.hpp"

static std::string get_file_content(const std::string& file_path)
{
	std::ifstream file(file_path.c_str());
	if (!file) throw std::runtime_error("Invalid config file (" + file_path + ")");
	std::ostringstream sstr;
	sstr << file.rdbuf();
	return sstr.str();
}

//clean comments and empty nl in file header
//return number of lines deleted
static std::size_t clean_file_head(std::string& str)
{
	std::size_t i = 0;
	while (1)
	{
		if (str[0] == '#')
		{
			std::size_t tmp = str.find('\n');
			if (tmp == std::string::npos) tmp = str.length() - 1;
			str = str.substr(tmp, str.length() - tmp);
		}
		if (str[0] == '\n')
			str = str.substr(1, str.length() - 1);
		else
			break;
		i++;
	}
	return i;
}

//i is index of location '{'
//will check if location block has valid syntax (i.e.: location /ok/cool { )
static bool is_valid_location(const std::string& str, std::size_t i, std::size_t ln2)
{
	std::size_t j = str.rfind("location", i);
	if (j == std::string::npos) throw std::runtime_error(to_string(ln2) + ": no location keyword before '{'");
	j += std::string("location").length();
	while (std::isspace(str[j])) j++;
	if (j == i) throw std::runtime_error(to_string(ln2) + ": need path between location and '{'");
	if (str[j] != '/') throw std::runtime_error(to_string(ln2) + ": location path must begin with a '/'");
	while (!std::isspace(str[j] && j != i)) j++;
	if (j == i) return true;
	while (std::isspace(str[j] && j != i)) j++;
	if (j == i) return true;
	throw std::runtime_error(to_string(ln2) + ": only whitespaces are allowed between location path and '{'");
}

//check if correct depth of braces in server and returns index of ending '}'
//i is index of beginning '{'
//also checks if location blocks are valid
static std::size_t get_end_server_block(const std::string& str, std::size_t i, std::size_t ln2)
{
	int depth = 0;
	while (++i < str.length())
	{
		if (str[i] == '\n') ln2++;
		if (str[i] == '}' && depth == 0) return i;
		if (str[i] == '}' && depth == 1) depth--;
		if (str[i] == '{' && depth == 1) throw std::runtime_error(to_string(ln2) + ": open bracket in location block");
		if (str[i] == '{' && depth == 0)
		{
			if (is_valid_location(str, i, ln2)) depth++;
			else throw std::runtime_error(to_string(ln2) + ": block inside server is not a location block");
		}
	}
	throw std::runtime_error(to_string(ln2) + ": server block not closed");
}

//check if braces are only server and location blocks
static void check_braces(const std::string& str, std::size_t line_num)
{
	std::size_t ln2 = line_num;
	std::size_t i = 0;
	while (i < str.length())
	{
		std::size_t j = std::string(&str[i]).find("server");
		if (j == std::string::npos) j = str.length();
		while (i < j) {
			if (str[i] == '\n') ln2++;
			if (!std::isspace(str[i])) throw std::runtime_error(to_string(ln2) + ": global statement is not 'server'");
			i++;
		}
		i += std::string("server").length();
		j = std::string(&str[i]).find("{");
		if (j == std::string::npos) j = str.length(); else j += i;
		while (i < j) {
			if (str[i] == '\n') ln2++;
			if (!std::isspace(str[i])) throw std::runtime_error(to_string(ln2) + ": global 'server' statement not followed by '{'");
			i++;
		}
		j = get_end_server_block(str, i, ln2); //i is on '{' here
		while (++i < j)
			ln2 += str[i] == '\n';
		i++; //to jump over '}'
	}
}

std::vector<config> init_configs(const std::string& file_path)
{
	std::string str = get_file_content(file_path);
	std::size_t line_num = clean_file_head(str);
	check_braces(str, line_num);
	//here server blocks and location blocks are good.
	//still need to fill vector and check if statements are valid.
	return std::vector<config>();
}

int main()
{
	std::string path = "./default.conf";
	try {
	init_configs(path);
	} catch (std::exception& e) {
		std::cerr << "Error in " + path + ":\n\t" + e.what() + "\n";
	}
}

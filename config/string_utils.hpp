#pragma once
#include <string>
#include <sstream>

template <class T>
std::string to_string(const T& p)
{
	std::stringstream ss;
	ss << p;
	return ss.str();
}

int my_stoi(const std::string& text);

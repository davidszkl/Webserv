#include <string>
#include <sstream>
#include <exception>

int my_stoi(const std::string& text)
{
	std::istringstream iss (text);
	int number;
	iss >> number;
	if (iss.fail()) {
		throw std::runtime_error("stoi failed");
	}
	return number;
}

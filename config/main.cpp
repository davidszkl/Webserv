#include "config.hpp"
#include "debug.hpp"

int main()
{
	std::string path = "./default.conf";
	std::vector<config> vec;
	try {
		vec = init_configs(path);
	} catch (std::exception& e) {
		std::cerr << "Error in " + path + ":\n\t" + e.what() + "\n";
	}
	for (std::size_t i = 0; i < vec.size(); i++) log(vec[i]);
}

#pragma once

#include <iostream>

template <class T>
void log(const T& s) {
#ifdef DEBUG
	std::cerr << s;
#else
	(void)s;
#endif
}

template <class T>
void logn(const T& s) {
	log(s);
	log('\n');
}




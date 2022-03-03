#pragma once
#include <string>

/*
	checks if http message (header + body) is valid for a cgi response.
	full_message can be the whole http message received from the client,
	but only the first line will be parsed (the line that contains REQUEST PATH PROTOCOL\r\n)
	root is the root of the server.
 */
bool is_valid_for_cgi(const std::string& full_message, const std::string& root);

/*
	This function needs the first line of the http header and the body, so pass the full http message in full_message.
	root is the root of the server.
	ouput_fd is where the python script will write its ouput.
	The http message should be valid! (is_valid_for_cgi must have returned true with the same message/root)
 */
void execute_cgi(const std::string& full_message, const std::string& root, int ouput_fd);


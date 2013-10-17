/*
 * string_find.cpp
 *
 *  Created on: 2013年10月14日
 *      Author: icejoywoo
 */

#include <string>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
	std::string s = "Alice Bob Charlie";
	size_t position;
	position = s.find("none");
	// if (position > 0) { // size_t 是无符号的， -1是其最大值， position永远大于0
	if (position != std::string::npos) {
		std::cout << "Found! position is: " << position << std::endl;
	} else {
		std::cout << "Not Found!" << std::endl;
	}
	return 0;
}

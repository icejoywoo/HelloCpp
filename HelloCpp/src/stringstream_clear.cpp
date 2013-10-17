/*
 * stringstream_clear.cpp
 *
 *  Created on: 2013年10月14日
 *      Author: icejoywoo
 */

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
	int size = 100;
	std::stringstream stream;
	for (int i = 0; i < size; i++) {
		stream << i;
		std::string num;
		stream >> num;
		stream.clear(); // 没有正真释放stream占用的空间，会一直增长
		stream.str("");
		std::cout << num << std::endl;
	}
	printf("size = %lu\n", stream.str().capacity());
	return 0;
}

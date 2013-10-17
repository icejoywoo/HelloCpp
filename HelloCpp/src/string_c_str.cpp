/*
 * string_c_str.cpp
 *
 *  Created on: 2013年10月14日
 *      Author: icejoywoo
 */

#include <string>
#include <cstring>
#include <cstdio>

int main(int argc, char* argv[]) {
	std::string str = "abcd";
	// const char* pcStr = str.c_str(); // 可能引起core或者乱码， pcStr现在是野指针， str重新分配了内存， 源地址可能被覆盖
	char pcStr[20];
	strcpy(pcStr, str.c_str());
	printf("cStr = %s, pcStr = %p\n", pcStr, pcStr);
	str.append("egh");
	const char* pcStr2 = str.c_str();
	printf("cStr2 = %s, pcStr2 = %p\n", pcStr2, pcStr2);
	printf("cStr = %s, pcStr = %p\n", pcStr, pcStr);
	return 0;
}

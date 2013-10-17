/*
 * sprintf_overflow.cpp
 *
 *  Created on: 2013年10月13日
 *      Author: icejoywoo
 */

#include <cstdio>

int main(int argc, char* argv[]) {
	char src[50] = "abcdefghijklmnopqrstuvwxyz %s";
	char buf[10] = "";
	printf("buf's len: %lu\n", sizeof(buf));
	// int len = sprintf(buf, "%s", src); // sprintf的src超过buf， 可能导致core
	int len = snprintf(buf, sizeof(buf), "%s", src);
	// int len = snprintf(buf, sizeof(buf), src); // 会core
	if ((unsigned long)len > sizeof(buf) - 1ul) {
		fprintf(stderr, "snprintf imcomplete.\n");
	} else {
		printf("src: %s\n", src);
		printf("len: %d\n", len);
		printf("buf: %s\n", buf);
	}

	char* dest;
	// 不需要指定大小，防止截断，第一个参数是char**
	len = asprintf(&dest, "%s", src);
	printf("src: %s\n", src);
	printf("len: %d\n", len);
	printf("buf: %s\n", dest);
	return 0;
}


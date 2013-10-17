/*
 * foef_trap.cpp
 *
 *  Created on: 2013年10月15日
 *      Author: icejoywoo
 */

#include <cstdio>

int main() {
	FILE* fp;
	char line[100];
	fp = fopen("/etc/passwd", "r");
	// 最后一行被读出两遍
//	while (!feof(fp)) {
//		fgets(line, sizeof(line), fp);
//		printf("%s", line);
//	}
	while (true) {
		fgets(line, sizeof(line), fp);
		if (!feof(fp)) {
			printf("%s", line);
		} else {
			break;
		}
	}
	fclose(fp);
	return 0;
}

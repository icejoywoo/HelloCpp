/*
 * foef_trap.cpp
 *
 *  Created on: 2013年10月15日
 *      Author: icejoywoo
 */

#include <cstdio>
#include <memory>

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
	std::auto_ptr<int> p(new int(3));
	printf("%d\n", *p.get());
	return 0;
}

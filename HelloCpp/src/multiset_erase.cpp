/*
 * multiset_erase.cpp
 *
 *  Created on: 2013年10月23日
 *      Author: icejoywoo
 */

#include <iostream>
#include <set>

int main() {
	std::multiset<int> cl;
	cl.insert(3);
	cl.insert(3);
	cl.insert(3);
	cl.insert(3);
	cl.insert(2);
	cl.insert(5);

	std::multiset<int>::iterator pos = cl.find(3);
	cl.erase(pos);
	//cl.erase(3); // 删除所有3的元素
	for (std::multiset<int>::iterator iter = cl.begin(); iter != cl.end(); iter++) {
		std::cout << *iter << std::endl;
	}
	return 0;
}

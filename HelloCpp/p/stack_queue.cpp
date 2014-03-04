/*
 * stack_queue.cpp
 *
 *  Created on: 2014年3月4日
 *      Author: icejoywoo
 */

#include <iostream>
#include <queue>
#include <stack>

template<typename T>
class MyQueue {
public:
	explicit MyQueue() {
	}
	virtual ~MyQueue() {
	}

	void push(T val) {
		second_stack.push(val);
	}

	void pop() {
		move(second_stack, first_stack);
		first_stack.pop();
	}

	T front() {
		move(second_stack, first_stack);
		return first_stack.top();
	}

	T back() {
		move(first_stack, second_stack);
		return second_stack.top();
	}

	bool empty() {
		return this->size() == 0;
	}

	size_t size() {
		return first_stack.size() + second_stack.size();
	}
private:
	void move(std::stack<T>& src, std::stack<T>& dst) {
		if (dst.empty()) {
			while (!src.empty()) {
				dst.push(src.top());
				src.pop();
			}
		}
	}

	std::stack<T> first_stack;
	std::stack<T> second_stack;
};

int main() {
	std::queue<int> q;
	MyQueue<int> mq;

	for (int i = 0; i < 10; ++i) {
		q.push(i);
		mq.push(i);
	}

	for (int i = 0; i < 10; ++i) {
		std::cout << q.front() << std::endl;
		q.pop();
	}

	std::cout << "====" << std::endl;

	for (int i = 0; i < 10; ++i) {
		std::cout << mq.front() << std::endl;
		mq.pop();
	}
}

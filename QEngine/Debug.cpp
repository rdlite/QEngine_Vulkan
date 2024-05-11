#include <iostream>
#include "Debug.h"

void Debug::print(const char* data) {
	std::cout << data << std::endl;
}

void Debug::print(int data) {
	std::cout << data << std::endl;
}

void Debug::print(float data) {
	std::cout << data << std::endl;
}

void Debug::print(std::string data) {
	std::cout << data << std::endl;
}

void Debug::print(uint32_t data) {
	std::cout << data << std::endl;
}

void Debug::print(size_t data) {
	std::cout << data << std::endl;
}

void Debug::print(bool data) {
	std::cout << (data ? "true" : "false") << std::endl;
}

void Debug::print(double data) {
	std::cout << data << std::endl;
}
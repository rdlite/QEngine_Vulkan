#pragma once
#include <iostream>

class ThrowErr {
public:
	static void runtime(const char* msg);
	static void runtime(std::string msg);
};
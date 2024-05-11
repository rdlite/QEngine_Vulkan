#include "ThrowErr.h"

void ThrowErr::runtime(const char* msg) {
	throw std::runtime_error(msg);
}

void ThrowErr::runtime(std::string msg) {
	throw std::runtime_error(msg);
}
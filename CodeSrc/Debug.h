#pragma once
class Debug {
public:
	static void print(const char* data);
	static void print(int data);
	static void print(float data);
	static void print(std::string data);
	static void print(uint32_t data);
	static void print(size_t data);
	static void print(bool data);
	static void print(double data);
};
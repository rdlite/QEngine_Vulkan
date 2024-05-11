#pragma once
#include <vector>
#include <iostream>
#include <cstring>  

class QString {
public:
	static std::string getDirname(std::string fullpath);
	static std::string getFilename(std::string fullpath);
	static std::vector<std::string> splitString(std::string s, std::string delimiter);
	static char* convertStringToCharNonConst(std::string str);
	static int len(std::string str);
	static bool contains(std::string str, std::string check);
};
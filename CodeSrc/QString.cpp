#include "QString.h"

std::string QString::getDirname(std::string fullpath) {
    if (!QString::contains(fullpath, "/")) {
        std::cout << "Path is incorrect..";
        return fullpath;
    }

    std::vector<std::string> parts = QString::splitString(fullpath, "/");

    if (QString::contains(parts[parts.size() - 1], ".")) {
        std::string result = "";

        for (int i = 0; i < parts.size() - 1; i++) {
            result += parts[i] + "/";
        }

        return result;
    }
    else {
        return fullpath;
    }
}

std::string QString::getFilename(std::string fullpath) {
    if (!QString::contains(fullpath, "/")) {
        std::cout << "Path is incorrect..";
        return fullpath;
    }

    std::vector<std::string> parts = QString::splitString(fullpath, "/");

    if (QString::contains(parts[parts.size() - 1], ".")) {
        return parts[parts.size() - 1];
    }
    else {
        return fullpath;
    }
}

std::vector<std::string> QString::splitString(std::string str, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
        token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(str.substr(pos_start));
    return res;
}

char* QString::convertStringToCharNonConst(std::string str) {
    char* cstr = new char[str.size() + 1];
    return cstr;
}

int QString::len(std::string str) {
    int length = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        length++;
    }
    return length;
}

bool QString::contains(std::string str, std::string check) {
    if (str.find(check) != std::string::npos) {
        return true;
    }

    return false;
}
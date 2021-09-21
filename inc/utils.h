#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <cstdlib>
#include <cstring>

class Utils {
public:
    static const std::string GetFileNameOnLinux(const std::string &file_path);
};

#endif
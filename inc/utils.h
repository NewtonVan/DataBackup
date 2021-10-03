#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <cstdlib>
#include <cstring>

class Utils {
public:
    static const std::string BaseName(const std::string &file_path);
    static const std::string ReNameBase(const std::string &file_path, const std::string &surfix);
    static unsigned char SetBit(unsigned char uc, int bit_index, int set_bit);
    static int GetBit(unsigned char uc, int bit_index);
};

#endif
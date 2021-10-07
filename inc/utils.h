#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "header.h"
#include "exception_interface.h"

#define CP_BLOCK_SIZE 512

class CopyException : public BaseException{
public:
    CopyException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg)
    {

    }
    std::string what()
    {
        return "Copy Exception : "+BaseException::what();
    }
};

class Utils {
public:
    static const std::string BaseName(const std::string &file_path);
    static const std::string ReNameBase(const std::string &file_path, const std::string &surfix);
    static unsigned char SetBit(unsigned char uc, int bit_index, int set_bit);
    static int GetBit(unsigned char uc, int bit_index);
    static void Copy(Header &h, int fd_src, int fd_dst, int padding_on);
};

#endif
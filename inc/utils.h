#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unistd.h>

#include "header.h"
#include "exception_interface.h"

#define CP_BLOCK_SIZE 512

using std::string;

class CopyException : public BaseException{
public:
    CopyException(const string &file_nm, const string &msg)
        : BaseException(file_nm, msg)
    {

    }
    string what()
    {
        return "Copy Exception : "+BaseException::what();
    }
};

class Utils {
public:
    static const string BaseName(const string &file_path);
    static const string ReNameBase(const string &file_path, const string &surfix);
    static unsigned char SetBit(unsigned char uc, int bit_index, int set_bit);
    static int GetBit(unsigned char uc, int bit_index);
    static void Copy(Header &h, int fd_src, int fd_dst, int padding_on);
    static void RegPath(char *const path);
    static void RecurMkdir(const string &dst);
};

#endif
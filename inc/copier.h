#ifndef COPIER_H
#define COPIER_H

#include <unistd.h>
#include <cstring>


#include "header.h"
#include "exception_interface.h"

#define CP_BLOCK_SIZE 512

void Copy(int fd_src, int fd_dst, int padding_on);

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

#endif
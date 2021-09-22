#ifndef COPIER_H
#define COPIER_H

#include <unistd.h>
#include <cstring>


#include "header.h"

#define CP_BLOCK_SIZE 512

void Copy(int fd_src, int fd_dst);

class CopyException{
private:
    std::string file_nm_;
    std::string msg_;
public:
    CopyException(const std::string &file_nm, const std::string &msg)
        : file_nm_(file_nm), msg_(msg)
    {

    }
    std::string ErrMsg()
    {
        return msg_;
    }
    std::string FileName()
    {
        return file_nm_;
    }
};


#endif
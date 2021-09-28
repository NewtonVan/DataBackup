#ifndef SHA_HASH_H_
#define SHA_HASH_H_

#include <openssl/sha.h>

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <iostream>

#include "exception_interface.h"

class ShaException : public BaseException{
public:
    ShaException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg) {}
    std::string what() const {
        return "Sha Exception : "+BaseException::what();
    }
};

class Sha256Hasher {
public:
    int Handle(const std::string &src, const std::string &dst);
private:
    void Init(const std::string &dst);
    void Hash(const std::string &src);
    void HashDir(const std::string &src, const struct stat &st_buf);
    void HashReg(const std::string &src, const struct stat &st_buf);
    void HashFifo(const std::string &src, const struct stat &st_buf);
    void HashLink(const std::string &src, const struct stat &st_buf);
    void WriteLine(const std::string &line);
    void Clear();
    void RestoreAccessTime(const std::string &src_file, const struct stat &st_buf);
private:
    void sha256_hash_string(char hash[SHA256_DIGEST_LENGTH], 
        char outputBuffer[65]);
    void sha256_string(const char *str, char outputBuffer[65]);
    void sha256_file(const char *path, char outputBuffer[65]);
private:
    std::string dst_file_;
    int dst_fd_;
    std::vector<BaseException> errs_;
};

#endif
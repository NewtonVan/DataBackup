#ifndef CIO_DECRYPT_H_
#define CIO_DECRYPT_H_

#include <openssl/md5.h>
#include <openssl/aes.h>

#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include <string>
#include <vector>
#include <cstring>

#include "handler.h"
#include "exception_interface.h"

#define MAX_BUF_SIZE 4096  // 必须是16的倍数

class DecryptException : public BaseException {
public:
    DecryptException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg) {}
    std::string what() {
        return "Decrypt Exception : "+BaseException::what();
    }
};

/**
 * 使用：务必先set pwd，然后进行handle
 */
class Decryptor {
public:
    int Handle(const std::string &src, const std::string &dst="");
    void SetPassword(const std::string &pwd) {
        pwd_ = pwd;
    }
// Todo: for test
// private:
public:
    void Init(const std::string &src_file);
    void CheckDigest();
    void Decrypt();
    void Clear();
private:
    std::string src_file_;
    int src_fd_;
    // Todo：大文件会不会有瓶颈
    off_t src_file_size_;
    std::string dst_file_;
    int dst_fd_;
    std::string pwd_;
    std::vector<BaseException> errs_;
};

#endif
#ifndef PACKER_H_
#define PACKER_H_

// linux
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>

// C++
#include <vector>
#include <string>
#include <unordered_set>
#include <cstring>

#include "exception_interface.h"
#include "header.h"
#include "handler.h"
#include "utils.h"

#define MY_BLOCK_SIZE 512
#define MAX_BUF_SIZE 4096

class PackException : public BaseException{
public:
    PackException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg) {}
    std::string what() const {
        return "Pack Exception : "+BaseException::what();
    }
};

/**
 * 输入：一个或多个文件的路径；一个目标路径
 * 操作：将所有文件打包至目标文件夹下
 */
class Packer : public BaseHandler,public ExceptionContainer{
public:
    int Handle(const std::string &src, const std::string &dst);
// Todo: 方便测试，暂public
// private:
public:
    void Pack(const std::string &src_file);
    void ParseHeader(const std::string &src_file, const struct stat &st_buf);
    void PackDir(const std::string &src_file, const struct stat &st_buf);
    void PackRegular(const std::string &src_file, const struct stat &st_buf);
    void PackLink(const std::string &src_file, const struct stat &st_buf);
    void RestoreAccessTime(const std::string &src_file, const struct stat &st_buf);
private:
    std::string abs_parent_path_;
    std::string dst_file_;
    int dst_fd_;
    std::unordered_set<nlink_t> hard_link_set_;
    Header header_;
};

#endif
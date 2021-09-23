#ifndef PACKER_H_
#define PACKER_H_

#include "header.h"

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

// define
// #define MY_BLOCK_SIZE 512
// #define MAX_BUF_SIZE 512

/**
 * 输入：一个或多个文件的路径；一个目标路径
 * 操作：将所有文件打包至目标文件夹下
 * 输出：
 * 
 * 多线程单例模式，一般要求对象没有状态信息；本类无状态信息，因此支持多线程操作
 * 但是一些api调用是不支持多线程的，例如检查文件存在与否，是有时间新鲜度的，最好不要多线程
 */
class Packer {
public:
    static Packer &GetInstance();
    int Pack(const std::vector<std::string> &src_files, const std::string &out_dir);
private:
    // 单例
    Packer() = default;
    ~Packer() = default;
    Packer(const Packer &p) = delete;
    const Packer &operator=(const Packer &p) = delete;
// todo: 方便测试，暂时用作public
// private:
public:
    int Pack(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd, std::unordered_set<nlink_t> &hard_link_set);
    // 注意本函数并不专门处理软链接
    void ParseHeader(const std::string &abs_parent_path, const std::string &src_file, const struct stat *st_buf_ptr);
    int PackDir(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd, const struct stat &st_buf, std::unordered_set<nlink_t> &hard_link_set);
    int PackRegular(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd, const struct stat &st_buf, std::unordered_set<nlink_t> &hard_link_set);
};

#endif
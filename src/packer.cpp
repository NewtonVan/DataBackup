#include "packer.h"
#include "header.h"

#include <iostream>

using namespace std;

/**
 * Todo：假设了输入、输出的路径均为绝对路径，且不为链接
 * 务必先set src_files
 * 
 * 打包处理：对于每个src_file，需得到父目录绝对路径，以得到目录项相对路径
 */
int Packer::Handle(const std::string &src, const std::string &dst) {
    try
    {
        // 1、Init工作
        if(src_files_.empty()) {
            throw PackException("", "bad src path");
        }
        if(0==access(dst.c_str(), F_OK)) {
            struct stat st_buf;
            stat(dst.c_str(), &st_buf);
            if(!S_ISDIR(st_buf.st_mode)) {
                throw PackException("", "dst path not a directory");
            }
        } else {
            // Todo：应该进行递归目录创建
            if(-1==mkdir(dst.c_str(), 0777)) {
                throw PackException("", "mkdir failed");
            }
        }
        // 创建backup文件：多源路径、一个源路径 命名方式不同
        if(src_files_.size()==1) {
            dst_file_ = dst + '/' + basename(src_files_[0].c_str()) + ".backup";
        } else {
            dst_file_ = dst + '/' + basename(dst.c_str()) + ".backup";
        }
        dst_fd_ = open(dst_file_.c_str(), O_CREAT | O_WRONLY);
        if(-1==dst_fd_) {
            throw PackException("", "failed to open backup file");
        }
        // 其他成员变量初始化
        hard_link_set_.clear();
        errs_.clear();

        // 2、打包
        for(const string &s : src_files_) {
            abs_parent_path_ = string(s.begin(), s.begin()+s.find_last_of('/')+1);
            Pack(s);
        }

        // extra: sha256 hash
        

        // 3、清理工作
        src_files_.clear();
        // if(-1 == close(src_fd_)) {
        //     throw PackException("", "failed to close src_fd");
        // }
        if(-1 == close(dst_fd_)) {
            throw PackException("", "failed to close dst_fd");
        }
    }
    catch(const BaseException &err)
    {
        errs_.push_back(err);
    }
    
    // TODO
    // Exception handle
    for (Exception &err : errs_){
        err.what();
        cout << err.what() << endl;
    }

    return 0;
}

void Packer::SetSrcFiles(const std::vector<std::string> &src_files) {
    src_files_ = src_files;
}

void Packer::Pack(const std::string &src_file) {
    // 检查src file有效性，小心解引用
    struct stat st_buf;
    if(-1 == lstat(src_file.c_str(), &st_buf)) {
        throw PackException("", "lstat failed on "+src_file);
    }
    ParseHeader(src_file, st_buf);
    if(S_ISDIR(st_buf.st_mode)) {
        PackDir(src_file, st_buf);
    } else if(S_ISREG(st_buf.st_mode)) {
        PackRegular(src_file, st_buf);
    } else if(S_ISFIFO(st_buf.st_mode)) {
        if(-1==header_.Serialize(dst_fd_)) {
            throw PackException("", "write header failed: " + src_file);
        }
    } else if(S_ISLNK(st_buf.st_mode)) { // 符号链接
        PackLink(src_file, st_buf);
    } else {
        // 其他类型文件，不需处理
    }
}

/**
 * 软链接等特殊字段需要额外处理
 */
void Packer::ParseHeader(const std::string &src_file, const struct stat &st_buf) {
    string file_path(src_file.begin() + abs_parent_path_.size(), src_file.end());
    header_.setFilePath(file_path);
    header_.setSymbol("");
    header_.setLenFilePath(file_path.size());
    header_.setLenSymbol(0);
    header_.setIno(st_buf.st_ino);
    header_.setMode(st_buf.st_mode);
    header_.setNumLink(st_buf.st_nlink);
    header_.setUid(st_buf.st_uid);
    header_.setGid(st_buf.st_gid);
    header_.setAccessTime(st_buf.st_atim);
    header_.setModifyTime(st_buf.st_mtim);

    ulong block_num = 0;
    uint padding = 0;
    if(S_ISREG(st_buf.st_mode)) {
        block_num = st_buf.st_size / MY_BLOCK_SIZE;
        uint temp = st_buf.st_size % MY_BLOCK_SIZE;
        if(temp!=0) {
            block_num++;
            padding = MY_BLOCK_SIZE - temp;
        }
    }
    header_.setNumBlock(block_num);
    header_.setPadding(padding);
}

void Packer::PackDir(const std::string &src_file, const struct stat &st_buf) {
    // header序列化
    if(-1 == header_.Serialize(dst_fd_)) {
        throw PackException("", "write header failed: " + src_file);
    }
    // 目录项递归处理
    DIR *dirp;
    dirent *dp;
    dirp = opendir(src_file.c_str());
    if(NULL == dirp) {
        throw PackException("", "opendir failed on " + src_file);
    }
    for(;;) {
        errno = 0;
        dp = readdir(dirp);
        if(dp==NULL) {
            break;
        }
        const string d_name(dp->d_name);
        if(d_name=="." || d_name=="..") {
            continue;
        }
        // 关键递归
        const string child_filename = src_file+'/'+d_name;
        Pack(child_filename);
    }
    if(0 != errno) {
        throw PackException("", "readdir failed on " + src_file);
    }
    // close dir fd
    if(-1 == closedir(dirp)) {
        throw PackException("", "closedir failed on " + src_file);
    }
    // 还原访问时间
    RestoreAccessTime(src_file, st_buf);
}

/**
 * 硬链接处理：判断stat结构体的st_nlink，大于1，则检查map，
 * 若不存在，加入set，写入backup；否则只写入头部信息
 */
void Packer::PackRegular(const std::string &src_file, const struct stat &st_buf) {
    // 硬链接处理
    bool is_hard_link = false;
    if(st_buf.st_nlink>1) {
        if(hard_link_set_.count(st_buf.st_nlink)==0) {
            hard_link_set_.insert(st_buf.st_nlink);
        } else {
            is_hard_link = true;
        }
    }
    if(-1==header_.Serialize(dst_fd_)) {
        throw PackException("", "write header failed: " + src_file);
    }
    // 不是硬链接，需写入数据块
    if(!is_hard_link) {
        int src_fd = open(src_file.c_str(), O_RDONLY);
        if(src_fd==-1) {
            throw PackException("", "open failed on " + src_file);
        }
        ulong block_num =  header_.getNumBlock();
        char data_block_buf[MY_BLOCK_SIZE];
        for(ulong i=0; i<block_num; i++) {
            ssize_t read_num = read(src_fd, data_block_buf, MY_BLOCK_SIZE);
            if(0>=read_num) {
                throw PackException("", "read failed on " + src_file);
            }
            if(i==block_num-1) {
                // 最后一个数据块需要置零
                for(uint i=read_num; i<MY_BLOCK_SIZE; i++) {
                    data_block_buf[i] = '\0';
                }
            }
            if(MY_BLOCK_SIZE != write(dst_fd_, data_block_buf, MY_BLOCK_SIZE)) {
                throw PackException("", "write failed: " + src_file);
            }
        }
        if(-1==close(src_fd)) {
            throw PackException("", "close failed on " + src_file);
        }
        // 还原访问时间
        RestoreAccessTime(src_file, st_buf);
    }
}

void Packer::PackLink(const std::string &src_file, const struct stat &st_buf) {
    // 设置符号链接
    char buf[PATH_MAX+1];
    ssize_t read_num = readlink(src_file.c_str(), buf, PATH_MAX);
    if(-1==read_num) {
        throw PackException("", "readlink failed on " + src_file);
    }
    buf[read_num] = '\0';
    string ln_path(buf);
    header_.setSymbol(ln_path);
    header_.setLenSymbol(ln_path.size());
    if(-1==header_.Serialize(dst_fd_)) {
        throw PackException("", "write header failed: " + src_file);
    }
}

void Packer::RestoreAccessTime(const std::string &src_file, const struct stat &st_buf) {
    timespec tm[2];
    tm[0] = st_buf.st_atim;
    tm[1].tv_nsec = UTIME_OMIT;
    if(utimensat(AT_FDCWD, src_file.c_str(), tm, AT_SYMLINK_NOFOLLOW)==-1) {
        throw PackException("", "utimenstat failed on " + src_file);
    }
}
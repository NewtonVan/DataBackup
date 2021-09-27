#include "packer.h"
#include "error_messager.h"
#include "utils.h"
#include "header.h"

using namespace std;

Packer &Packer::GetInstance() {
    static Packer p;
    return p;
}

int Packer::Pack(const std::vector<std::string> &src_files, const std::string &out_dir) {
    // 输入检查，todo：暂不考虑相对路径，假设参数均为绝对路径。后面优化全部转换为绝对路径
    //（注意还要去除末尾的/，如果有的话
    // 源路径
    if(src_files.empty()) {
        ErrorMessager::PrintError(cout, "源路径为空");
        return -1;
    }
    // 目标路径
    // 先判断路径是否存在，不存在则进行创建，存在则进行文件、目录判断
    // 暂不考虑out_dir为链接的情况
    if(access(out_dir.c_str(), F_OK)==0) {
        struct stat st_buf;
        stat(out_dir.c_str(), &st_buf);
        if(!S_ISDIR(st_buf.st_mode)) {
            ErrorMessager::PrintError(cout, "目标路径并非目录");
            return -1;
        }
    } else {
        // 路径不存在
        // todo:
        // mkdir仅仅创建最后一个名字目录，需要前面目录名均存在
        // 若为健壮性，需要进一步处理
        if(mkdir(out_dir.c_str(), 0777)==-1) {
            ErrorMessager::PrintError(cout, out_dir + ": out_dir create failed", errno);
            return -1;
        }
    }

    // 现假设源路径、目标路径均无存在性问题
    // 分两种情况创建backup文件，多个源路径，一个源路径
    string backup_filename;
    if(src_files.size()==1) {
        backup_filename = out_dir + '/' + Utils::GetFileNameOnLinux(src_files[0]) + ".backup";
    } else { // 多个文件
        backup_filename = out_dir + '/' + Utils::GetFileNameOnLinux(out_dir) + ".backup";
    }
    // cout << backup_filename << endl;
    // cout << src_files.size() << ' ' << src_files[0] << endl;
    int backup_fd = open(backup_filename.c_str(), O_CREAT | O_WRONLY);
    if(backup_fd==-1) {
        ErrorMessager::PrintError(cout, "backup file create failed", errno);
        return -1;
    }

    /** 依次对源路径进行打包处理
     * 对于每个src_file，需要得到其父目录的绝对路径，用以得到目录项相对路径
     * 举例，要备份 /a/b/c/d/e，父目录为/a/b/c/d/
     * 
     * 另外，需要set处理硬链接（注意unpacker里面，需要的是map，因为需要硬链接的还原
     */
    unordered_set<nlink_t> hard_link_set;
    for(const string &s : src_files) {
        const string abs_parent_path(s.begin(), s.begin()+s.find_last_of('/')+1);
        if(Pack(abs_parent_path, s, backup_fd, hard_link_set)==-1) {
            ErrorMessager::PrintError(cout, s+" 打包失败");
            // todo: 需要进行清理工作
            return -1;
        }
    }

    if(close(backup_fd)==-1) {
        ErrorMessager::PrintError(cout, "close backup file failed", errno);
        return -1;
    }

    return 0;
}

int Packer::Pack(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd, unordered_set<nlink_t> &hard_link_set) {
    // // 先检查src file有效性
    // // 有问题，因为access直接对符号链接解引用，无法判断存在与否信息
    // if(access(src_file.c_str(), F_OK)==-1) {
    //     ErrorMessager::PrintError(cout, "源路径 "+src_file+" 不存在");
    //     return -1;
    // }

    // 判断是目录还是文件
    struct stat st_buf;
    if(lstat(src_file.c_str(), &st_buf) == -1) {
        if(errno == ENOENT) { // no entry
            ErrorMessager::PrintError(cout, "源路径 " + src_file + " 不存在");
        } else {
            ErrorMessager::PrintError(cout, "lstat failed on " + src_file, errno);
        }
        return -1;
    }
    if(S_ISDIR(st_buf.st_mode)) { // 是目录
        if(PackDir(abs_parent_path, src_file, backup_fd, st_buf, hard_link_set)==-1) {
            ErrorMessager::PrintError(cout, src_file+" 打包失败");
            return -1;
        }
        return 0;
    } else if(S_ISREG(st_buf.st_mode)) { // 是文件（需考虑硬链接处理
        if(PackRegular(abs_parent_path, src_file, backup_fd, st_buf, hard_link_set)==-1) {
            ErrorMessager::PrintError(cout, src_file+" 打包失败");
            return -1;
        }
        return 0;
    } else if(S_ISFIFO(st_buf.st_mode)) { // 有名管道
        ParseHeader(abs_parent_path, src_file, &st_buf);
        if(Header::GetInstance().Serialize(backup_fd) == -1) {
            ErrorMessager::PrintError(cout, src_file + " write header failed");
            return -1;
        }
    } else if(S_ISLNK(st_buf.st_mode)) { // 符号链接
        ParseHeader(abs_parent_path, src_file, &st_buf);

        // 设置符号链接
        char buf[PATH_MAX+1];
        ssize_t read_num = readlink(src_file.c_str(), buf, PATH_MAX);
        if(read_num==-1) {
            ErrorMessager::PrintError(cout, "readlink failed on " + src_file, errno);
            return -1;
        }
        buf[read_num] = '\0';
        string ln_path(buf);
        Header &h = Header::GetInstance();
        h.setSymbol(ln_path);
        h.setLenSymbol(ln_path.size());

        if(h.Serialize(backup_fd) == -1) {
            ErrorMessager::PrintError(cout, src_file + " write header failed");
            return -1;
        }
    } else {
        // 其他类型文件，不需处理
    }

    return 0;
}

void Packer::ParseHeader(const std::string &abs_parent_path, const std::string &src_file, const struct stat *st_buf_ptr) {
    Header &header = Header::GetInstance();

    // get file path
    string file_path(src_file.begin() + abs_parent_path.size(), src_file.end());
    header.setFilePath(file_path);

    // 不处理软链接
    header.setSymbol("");

    header.setLenFilePath(file_path.size());
    header.setLenSymbol(0);
    header.setIno(st_buf_ptr->st_ino);
    header.setMode(st_buf_ptr->st_mode);
    header.setNumLink(st_buf_ptr->st_nlink);
    header.setUid(st_buf_ptr->st_uid);
    header.setGid(st_buf_ptr->st_gid);
    header.setAccessTime(st_buf_ptr->st_atim);
    header.setModifyTime(st_buf_ptr->st_mtim);

    // 仅普通文件包含数据块
    ulong block_num = 0;
    uint padding = 0;
    if(S_ISREG(st_buf_ptr->st_mode)) {
        block_num = st_buf_ptr->st_size / MY_BLOCK_SIZE;
        uint temp = st_buf_ptr->st_size % MY_BLOCK_SIZE;
        if(temp!=0) {
            block_num++;
            padding = MY_BLOCK_SIZE - temp;
        }
    }
    header.setNumBlock(block_num);
    header.setPadding(padding);
}

int Packer::PackDir(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd, const struct stat &st_buf, unordered_set<nlink_t> &hard_link_set) {
    // 1、读入目录元数据到header，并将header序列化到backup文件
    ParseHeader(abs_parent_path, src_file, &st_buf);
    if(Header::GetInstance().Serialize(backup_fd) == -1) {
        ErrorMessager::PrintError(cout, src_file + " write header failed");
        return -1;
    }

    // 2、读取目录的数据（目录项），对每个目录项进行递归处理
    // 注意需要暂存访问时间，然后在之后恢复
    timespec tm[2];
    tm[0] = st_buf.st_atim;
    tm[1].tv_nsec = UTIME_OMIT;

    DIR *dirp;
    dirent *dp;
    dirp = opendir(src_file.c_str());
    if(dirp == NULL) {
        ErrorMessager::PrintError(cout, "opendir failed on " + src_file, errno);
        return -1;
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
        if(Pack(abs_parent_path, child_filename, backup_fd, hard_link_set)==-1) {
            ErrorMessager::PrintError(cout, child_filename + " 打包失败");
            return -1;
        }
    }
    if(errno != 0) {
        ErrorMessager::PrintError(cout, "readdir failed", errno);
        return -1;
    }
    
    // close dir描述符，并还原访问时间
    if(closedir(dirp)==-1) {
        ErrorMessager::PrintError(cout, "closedir failed on " + src_file, errno);
        return -1;
    }
    if(utimensat(AT_FDCWD, src_file.c_str(), tm, AT_SYMLINK_NOFOLLOW)==-1) {
        ErrorMessager::PrintError(cout, src_file + " utimenstat failed", errno);
        return -1;
    }
    return 0;
}

int Packer::PackRegular(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd, const struct stat &st_buf, unordered_set<nlink_t> &hard_link_set) {
    /**
     * 硬链接处理：判断stat结构体的st_nlink
     * 若st_nlink数大于1，检查map（key为st_ino，value为路径名），若不存在，加入set，写入backup；
     * 否则，只写入头部信息
     */
    bool is_hard_link = false;
    if(st_buf.st_nlink>1) {
        if(hard_link_set.count(st_buf.st_nlink)==0) { // 第一次出现
            hard_link_set.insert(st_buf.st_nlink);
        } else { // 是硬链接
            is_hard_link = true;
        }
    }

    Header &h = Header::GetInstance();
    ParseHeader(abs_parent_path, src_file, &st_buf);
    if(h.Serialize(backup_fd) == -1) {
        ErrorMessager::PrintError(cout, src_file + " write header failed");
        return -1;
    }

    if(!is_hard_link) { // 不是硬链接，需写入数据块
        // 需要保存access time，之后还原
        timespec tm[2];
        tm[0] = st_buf.st_atim;
        tm[1].tv_nsec = UTIME_OMIT;

        int src_fd = open(src_file.c_str(), O_RDONLY);
        if(src_fd==-1) {
            ErrorMessager::PrintError(cout, "open failed on " + src_file, errno);
            return -1;
        }
        ulong block_num =  h.getNumBlock();
        char data_block_buf[MY_BLOCK_SIZE];
        for(ulong i=0; i<block_num; i++) {
            ssize_t read_num = read(src_fd, data_block_buf, MY_BLOCK_SIZE);
            if(read_num==-1) {
                ErrorMessager::PrintError(cout, "read failed on " + src_file, errno);
                return -1;
            }

            if(i==block_num-1) {
                // 最后一个数据块需要置零
                for(uint i=read_num; i<MY_BLOCK_SIZE; i++) {
                    data_block_buf[i] = '\0';
                }
            }

            if(write(backup_fd, data_block_buf, MY_BLOCK_SIZE) != MY_BLOCK_SIZE) {
                ErrorMessager::PrintError(cout, "write failed on " + src_file, errno);
                return -1;
            }
        }
        if(close(src_fd)==-1) {
            ErrorMessager::PrintError(cout, "close failed on " + src_file, errno);
            return -1;
        }
        if(utimensat(AT_FDCWD, src_file.c_str(), tm, AT_SYMLINK_NOFOLLOW)==-1) {
            ErrorMessager::PrintError(cout, src_file + " utimenstat failed", errno);
            return -1;
        }
    }

    return 0;
}
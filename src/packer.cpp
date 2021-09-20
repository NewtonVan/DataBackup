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
    // 输入检查，todo：暂不考虑相对路径，假设参数均为绝对路径
    // 源路径
    if(src_files.empty()) {
        ErrorMessager::PrintError(cout, "源路径为空");
        return -1;
    }
    for(const string &s : src_files) {
        
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
        if(mkdir(out_dir.c_str(), 0777)!=0) {
            ErrorMessager::PrintError(cout, "目标文件夹创建失败");
            return -1;
        }
    }

    // 现假设源路径、目标路径均无存在性问题
    // 分两种情况创建backup文件，多个源路径，一个源路径
    string backup_filename;
    if(src_files.size()==1) {
        backup_filename = Utils::GetFileNameOnLinux(src_files[0]) + ".backup";
    } else { // 多个文件
        backup_filename = Utils::GetFileNameOnLinux(out_dir) + ".backup";
    }
    int backup_fd = open(backup_filename.c_str(), O_CREAT | O_WRONLY);
    if(backup_fd==-1) {
        ErrorMessager::PrintError(cout, "备份文件创建失败 errno: " + to_string(errno));
        return -1;
    }

    // 依次对源路径进行打包处理
    // 对于每个src_file，需要得到其父目录的绝对路径，用以得到目录项相对路径
    for(const string &s : src_files) {
        const string abs_parent_path(s.begin(), s.begin()+s.find_last_of('/')+1);
        if(Pack(abs_parent_path, s, backup_fd)==-1) {
            ErrorMessager::PrintError(cout, s+" 打包失败");
            // todo: 需要进行清理工作
            return -1;
        }
    }

    close(backup_fd);
}

int Packer::Pack(const std::string &abs_parent_path, const std::string &src_file, const int backup_fd) {
    // 先检查src file有效性
    if(access(src_file.c_str(), F_OK)!=0) {
        ErrorMessager::PrintError(cout, "源路径 "+src_file+" 不存在");
        return -1;
    }

    // 判断是目录还是文件
    struct stat st_buf;
    stat(src_file.c_str(), &st_buf);
    if(S_ISDIR(st_buf.st_mode)) { // 是目录
        // 1、读入目录元数据到header，并将header序列化到backup文件
        

        // 2、读取目录的数据（目录项），对每个目录项进行递归处理
        // 3、关闭目录fd

    } else if(S_ISREG(st_buf.st_mode)) { // 是文件（需考虑硬链接处理

    } else if(S_ISFIFO(st_buf.st_mode)) { // 有名管道

    } else if(S_ISLNK(st_buf.st_mode)) { // 符号链接

    } else {
        // 其他类型文件，不需处理
    }
}

int Packer::ParseHeader(const std::string &abs_parent_path, const std::string &src_file, struct stat *st_buf) {
    Header &header = Header::GetInstance();
}
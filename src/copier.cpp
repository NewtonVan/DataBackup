#include "inc/copier.h"


using namespace std;

Copier &Copier::GetCopier() {
    static Copier copier;
    return copier;
}

void Copier::Copy(const std::vector<std::string> &src_files, const std::string &out_dir) {
    // 输入检查
    if(src_files.empty()) {

    }

    // 输出检查
    // 先判断路径是否存在，不存在则进行创建，存在则进行文件、目录判断
    // 暂不考虑out_dir为链接的情况
    if(access(out_dir.c_str(), F_OK)==0) {
        struct stat st_buf;
        stat(out_dir.c_str(), &st_buf);
        if(!S_ISDIR(st_buf.st_mode)) {

        }
    } else {
        if()
    }
}

void Copier::Copy(const std::string &src_file, std::string &out_dir) {

}
///
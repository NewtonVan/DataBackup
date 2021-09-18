#ifndef COPIER_H_
#define COPIER_H_

// linux
#include <unistd.h>
#include <sys/stat.h>

// C++
#include <vector>
#include <string>



/**
 * 输入：一个或多个文件的路径；一个目标路径
 * 操作：将所有文件从一个文件，拷贝到目标文件下
 * 输出：拷贝信息
 * 
 * 多线程单例模式，一般要求对象没有状态信息；本类无状态信息，因此支持多线程操作
 * 但是一些api调用是不支持多线程的，例如检查文件存在与否，是有时间新鲜度的，最好不要多线程
 */
class Copier {
public:
    static Copier &GetCopier();
    void Copy(const std::vector<std::string> &src_files, const std::string &out_dir);
private:
    // 单例模式支持
    Copier() = default;
    ~Copier() = default;
    Copier(const Copier &copier) = delete;
    const Copier &operator=(const Copier &copier) = delete;
private:
    void Copy(const std::string &src_file, std::string &out_dir);
// private:
//     std::vector<std::string> source_files_;
//     std::string output_dir_;
};

#endif
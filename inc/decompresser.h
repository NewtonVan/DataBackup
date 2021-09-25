#ifndef DECOMPRESSER_H_
#define DECOMPRESSER_H_

#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include <string>

#include "huffman_tree.h"

#define MAX_BUF_SIZE 4096

class DecompresseException {
public:
    DecompresseException(const std::string &msg) : msg_(msg) {}
    std::string ErrMsg() const {
        return msg_;
    }
private:
    std::string msg_;
};


class Decompresser
{
public:
    static Decompresser &GetInstance();
    void Decompress(const std::string &src_file);
private:
    Decompresser() = default;
    ~Decompresser() = default;
    Decompresser(const Decompresser &dc) = delete;
    const Decompresser &operator=(const Decompresser &dc) = delete;
private:
    void Init(const std::string &src_file);
    void ReadHeader();
    void ReadData();
    void Clear();
private:
    std::string src_file_;
    int src_fd_;
    std::string dst_file_;
    int dst_fd_;
    unsigned int last_byte_effective_; 
    std::vector<TreeNode *> counts_;
    TreeNode *huff_root_;
    std::vector<DecompresseException> errs_;
};

#endif
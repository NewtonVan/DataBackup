#ifndef COMPRESSER_H_
#define COMPRESSER_H_

// Linux
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

// C++
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <stack>

class CompresseException {
public:
    CompresseException(const std::string &msg) : msg_(msg) {}
    std::string ErrMsg() const {
        return msg_;
    }
private:
    std::string msg_;
};


class TreeNode {
public:
    TreeNode() 
        : left_(nullptr), right_(nullptr), ch_('\0'), weight_(0), code_("") {}
    TreeNode(unsigned char ch) 
        : left_(nullptr), right_(nullptr), ch_(ch), weight_(0), code_("") {}
    static TreeNode *BuildHuffman(const std::vector<TreeNode *> &counts);
    static bool Cmp(const TreeNode *x, const TreeNode *y) { // 用于优先队列
        return x->weight_ > y->weight_;
    }
private:
    static void GenerateCoding(TreeNode *root, std::string &code);
public:
    TreeNode *left_;
    TreeNode *right_;
    unsigned char ch_;
    uint64_t weight_;
    std::string code_;
};


class Compresser {
public:
    static Compresser &GetInstance();
    int Compress(const std::string &src_file);
private:
    Compresser() = default;
    ~Compresser() = default;
    Compresser(const Compresser &c) = delete;
    const Compresser &operator=(const Compresser &c) = delete;
// Todo: for test
// private:
public:
    void Init(const std::string &src_file);
    void WordFreqCount();
    void GenerateCodingTable();
    void WriteHeader();
    void WriteData();
    void Clear();
// Todo: for test
// private:
public:
    std::string src_file_;
    int src_fd_;
    std::string dst_file_;
    int dst_fd_;
    std::vector<TreeNode *> counts_;
    std::unordered_map<unsigned char, std::string> coding_table_;
    std::vector<CompresseException> errs_;
};

#endif
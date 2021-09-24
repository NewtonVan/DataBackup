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
#include <unordered_map>

#include "huffman_tree.h"

class CompresseException {
public:
    CompresseException(const std::string &msg) : msg_(msg) {}
    std::string ErrMsg() const {
        return msg_;
    }
private:
    std::string msg_;
};


class  Compresser {
public:
    static Compresser &GetInstance();
    void Compress(const std::string &src_file);
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
    unsigned int last_byte_effective_;
    std::vector<CompresseException> errs_;
};

#endif
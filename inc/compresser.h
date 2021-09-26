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
#include "exception_interface.h"

#define MAX_BUF_SIZE 4096

class CompresseException : public BaseException {
public:
    CompresseException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg) {}
    std::string what() {
        return "Compresse Exception : "+BaseException::what();
    }
};


class  Compresser {
public:
    int Handle(const std::string &src, const std::string &dst);
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
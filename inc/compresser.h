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
    std::string what() const {
        return "Compresse Exception : "+BaseException::what();
    }
};


class  Compresser : public ExceptionContainer{
public:
    int Handle(const std::string &src, const std::string &dst);
private:
    void Init(const std::string &src, const std::string &dst);
    void WordFreqCount();
    void GenerateCodingTable();
    void WriteHeader();
    void WriteData();
    void Clear();

private:
    std::string src_file_;
    int src_fd_;
    std::string dst_file_;
    int dst_fd_;
    std::vector<TreeNode *> counts_;
    std::unordered_map<unsigned char, std::string> coding_table_;
    unsigned int last_byte_effective_;
};

#endif
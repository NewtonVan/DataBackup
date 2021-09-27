#include "decompresser.h"
#include "utils.h"

using namespace std;

int Decompresser::Handle(const std::string &src, const std::string &dst) {
    try
    {
        Init(src, dst);
        ReadHeader();
        ReadData();
        Clear();
    }
    catch(const DecompresseException& e)
    {
        errs_.push_back(e);
    }

    return 0;
}

void Decompresser::Init(const std::string &src, const std::string &dst) {
    // 检查src_file
    if(src.empty()) {
        throw DecompresseException("", "bad path");
    }
    struct stat st_buf;
    if(-1 == lstat(src.c_str(), &st_buf)) {
        if(errno == ENOENT) { // no entry
            throw DecompresseException("", "no such file or directory: " + src);
        } else {
            throw DecompresseException("", "lstat failed on " + src);
        }
    }
    if(!S_ISREG(st_buf.st_mode)) {
        throw DecompresseException("", "not a regular file: " + src);
    }
    src_file_ = src;

    // 打开src_file
    src_fd_ = open(src_file_.c_str(), O_RDONLY);
    if(-1 == src_fd_) {
        throw DecompresseException("", "open failed on " + src_file_);
    }

    // 打开dst_file
    // Todo：解压文件名称
    dst_file_ = dst;
    dst_fd_ = open(dst_file_.c_str(), O_WRONLY | O_CREAT, 0777);
    if(-1 == dst_fd_) {
        throw DecompresseException("", "open failed on " + dst_file_);
    }

    // 成员变量init    
    counts_ = vector<TreeNode *>(256, nullptr);
    errs_.clear();
}

void Decompresser::ReadHeader() {
    unsigned int word_count;
    if(
        sizeof(word_count) != 
            read(src_fd_, &word_count, sizeof(word_count)) || 
        sizeof(last_byte_effective_) != 
            read(src_fd_, &last_byte_effective_, sizeof(last_byte_effective_))
    ) {
        throw DecompresseException("", "read failed on " + src_file_);
    }
    unsigned char char_buf;
    uint64_t weight_buf;
    for(unsigned int i=0; i<word_count; i++) {
        if(
            sizeof(char_buf) != read(src_fd_, &char_buf, sizeof(char_buf)) || 
            sizeof(weight_buf) != read(src_fd_, &weight_buf, sizeof(weight_buf))
        ) {
            throw DecompresseException("", "read failed on " + src_file_);
        }
        counts_[char_buf] = new TreeNode(char_buf);
        counts_[char_buf]->weight_ = weight_buf;
    }
}

/**
 * 压缩文件data结构：“末尾byte” + 正常顺序byte
 * 在last_byte_effective_不为0时需要考虑last byte处理
 */
void Decompresser::ReadData() {
    // 构建哈夫曼树
    huff_root_ = TreeNode::BuildHuffman(counts_);
    if(nullptr == huff_root_) {
        throw DecompresseException("", "failed to build huffman tree");
    }
    // 先读取“末尾byte”
    unsigned char last_byte;
    if(-1 == read(src_fd_, &last_byte, sizeof(last_byte))) {
        throw DecompresseException("", "read failed on " + src_file_);
    }
    // 读取data，每读到一位则在树上移动一位
    unsigned char read_buf[MAX_BUF_SIZE];
    unsigned char write_buf[MAX_BUF_SIZE];
    int write_buf_index = 0;
    size_t read_ret = 0;
    TreeNode *node = huff_root_;
    while(true) {
        read_ret = read(src_fd_, read_buf, MAX_BUF_SIZE);
        if(0 == read_ret) {
            break;
        }
        if(-1 == read_ret) {
            throw DecompresseException("", "read failed on " + src_file_);
        }
        for(ssize_t i=0; i<read_ret; i++) {
            unsigned char byte_buf = read_buf[i];
            for(int j=0; j<8; j++) {
                int b = Utils::GetBit(byte_buf, j);
                if(0 == b) {
                    node = node->left_;
                } else if(1 == b) {
                    node = node->right_;
                }
                if(nullptr==node->left_ && nullptr==node->right_) {
                    write_buf[write_buf_index] = node->ch_;
                    write_buf_index = (write_buf_index+1)%MAX_BUF_SIZE;
                    if(0 == write_buf_index) {
                        if(MAX_BUF_SIZE != write(dst_fd_, write_buf, MAX_BUF_SIZE)) {
                            throw DecompresseException("", "write failed on " + dst_file_);
                        }
                    }
                    // 重置node为根
                    node = huff_root_;
                }
            }
        }
    }
    // Todo：如何复用代码？
    for(int j=0; j<last_byte_effective_; j++) {
        int b = Utils::GetBit(last_byte, j);
        if(0 == b) {
            node = node->left_;
        } else if(1 == b) {
            node = node->right_;
        }
        if(nullptr==node->left_ && nullptr==node->right_) {
            write_buf[write_buf_index] = node->ch_;
            write_buf_index = (write_buf_index+1)%MAX_BUF_SIZE;
            if(0 == write_buf_index) {
                if(MAX_BUF_SIZE != write(dst_fd_, write_buf, MAX_BUF_SIZE)) {
                    throw DecompresseException("", "write failed on " + dst_file_);
                }
            }
            // 重置node为根
            node = huff_root_;
        }
    }
    if(write_buf_index != write(dst_fd_, write_buf, write_buf_index)) {
        throw DecompresseException("", "write failed on " + dst_file_);
    }
}

void Decompresser::Clear() {
    if(-1 == close(src_fd_)) {
        throw DecompresseException("", "failed to close src_fd");
    }

    if(-1 == close(dst_fd_)) {
        throw DecompresseException("", "failed to close dst_fd");
    }
}
#include "compresser.h"
#include "utils.h"

using namespace std;

/** 
 * 1、srcfile检查
 * 2、词频统计，得到词频数组（数组存储哈夫曼树指针）
 * 3、构建哈夫曼树，得到字符编码表（ 字符-编码）
 * 4、写入文件头：压缩文件标识（可选）、字符频度表、最后一个字节有效位数
 * 5、逐字符进行写入
 * 6、清理工作
 */
int Compresser::Handle(const std::string &src, const std::string &dst) {
    int err_code = 0;
    try
    {
        Init(src, dst);
        WordFreqCount();
        GenerateCodingTable();
        WriteHeader();
        WriteData();
        Clear();
        // TODO
        // refactor
        err_code = BaseHandler::Handle(dst_file_, dst);
    }
    catch(BaseException *err)
    {
        errs_.push_back(shared_ptr<BaseException>(err));
    }

    // TODO
    // refactor
    err_code = ExceptionContainer::ShowErrs();

    return err_code;
}

/**
 * 压缩文件名称：例如src名为a.backup，则dst命名为a.backup.cmps
 */
void Compresser::Init(const std::string &src_file, const std::string &dst) {
    // 检查src_file
    if(src_file.empty()) {
        throw new CompresseException("", "bad path");
    }
    struct stat st_buf;
    if(-1 == lstat(src_file.c_str(), &st_buf)) {
        if(errno == ENOENT) { // no entry
            throw new CompresseException("", "no such file or directory: " + src_file);
        } else {
            throw new CompresseException("", "lstat failed on " + src_file);
        }
    }
    if(!S_ISREG(st_buf.st_mode)) {
        throw new CompresseException("", "not a regular file: " + src_file);
    }
    src_file_ = src_file;

    // 打开src_file
    src_fd_ = open(src_file_.c_str(), O_RDONLY);
    if(-1 == src_fd_) {
        throw new CompresseException("", "open failed on " + src_file_);
    }

    // 打开dst_file
    // TODO 
    // refactor
    if (IsEnd()){
        dst_file_ = dst+"/"+Utils::ReNameBase(src_file, "cmp");
    } else{
        dst_file_ = "/tmp/"+Utils::ReNameBase(src_file, "cmp");
    }
    dst_fd_ = open(dst_file_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(-1 == dst_fd_) {
        throw new CompresseException("", "open failed on " + dst_file_);
    }

    // 成员变量init
    counts_ = vector<TreeNode *>(256, nullptr);
    coding_table_.clear();
    errs_.clear();
}

void Compresser::WordFreqCount() {
    unsigned char buf[MAX_BUF_SIZE];
    while(true) {
        ssize_t read_ret = read(src_fd_, buf, MAX_BUF_SIZE);
        if(0 == read_ret) {
            break;
        }
        if(-1 == read_ret) {
            throw new CompresseException("", "read failed on " + src_file_);
        }
        for(ssize_t i = 0; i<read_ret; i++) {
            if(nullptr == counts_[buf[i]]) {
                counts_[buf[i]] = new TreeNode(buf[i]);
            }
            counts_[buf[i]]->weight_++;
        }
    }
    // 若仅一个结点，哈夫曼树不好处理
    // 因此，额外添加一个结点，频度为0
    // Todo：是空文件又如何处理？
    unsigned int ch = 0;
    int word_count = 0;
    for(TreeNode *t : counts_) {
        if(nullptr != t) {
            ch = t->ch_;
            word_count++;
            if(word_count>1) break;
        }
    }
    if(word_count==1) {
        unsigned char new_ch = (unsigned char)(ch+1)%256;
        counts_[new_ch] = new TreeNode(new_ch);
    }
}

void Compresser::GenerateCodingTable() {
    TreeNode *root = TreeNode::BuildHuffman(counts_);
    if(nullptr == root) {
        throw new CompresseException("", "failed to build huffman tree");
    }
    stack<TreeNode *> help;
    help.push(root);
    while(!help.empty()) {
        TreeNode *node = help.top();
        help.pop();
        if(nullptr == node->left_ || nullptr == node->right_) {
            coding_table_[node->ch_] = node->code_;
            continue;
        }
        help.push(node->right_);
        help.push(node->left_);
    }
}

/**
 * 压缩文件header从简：词频个数 + 末尾byte有效位数
 */
void Compresser::WriteHeader() {
    unsigned int word_count = 0;
    for(TreeNode *t : counts_) {
        if(nullptr!=t) word_count++;
    }
    if(sizeof(word_count) != write(dst_fd_, &word_count, sizeof(word_count))) {
        throw new CompresseException("", "write failed on " + dst_file_); 
    }

    // 计算末尾byte有效位数
    unsigned int last_byte_effective = 0;
    for(TreeNode *t : counts_) {
        if(nullptr != t) {
            last_byte_effective = ( last_byte_effective + 
                (unsigned int)(t->weight_%8 * coding_table_[t->ch_].size()%8)%8 ) % 8;
        }
    }
    if(
        sizeof(last_byte_effective) != 
            write(dst_fd_, &last_byte_effective, sizeof(last_byte_effective))
    ) {
        throw new CompresseException("", "write failed on " + dst_file_); 
    }
    last_byte_effective_ = last_byte_effective;

    for(TreeNode *t : counts_) {
        if(nullptr!=t) {
            if(
                sizeof(t->ch_)!= write(dst_fd_, &t->ch_, sizeof(t->ch_)) || 
                sizeof(t->weight_)!=write(dst_fd_, &t->weight_, sizeof(t->weight_))
            ) {
                throw new CompresseException("", "write failed on " + dst_file_); 
            }
        }
    }
}

/**
 * 由于“末尾byte”可能补零，因此data结构：“末尾byte” + 正常顺序byte
 */
void Compresser::WriteData() {
    // 对src_fd进行lseek
    if(-1 == lseek(src_fd_, 0, SEEK_SET)) {
        throw new CompresseException("", "failed to lseek src_fd");
    }
    unsigned char read_buf[MAX_BUF_SIZE];
    unsigned char write_buf[MAX_BUF_SIZE];
    int write_buf_index = 0;
    unsigned char byte_buf = 0;
    int bit_index = 0;
    // Todo：是否需要考虑2G以上大文件？
    // 暂存“写入末尾byte”的位置
    off_t last_byte_pos;
    if(
        -1 == (last_byte_pos = lseek(dst_fd_, 0, SEEK_CUR)) || 
        -1 == lseek(dst_fd_, sizeof(byte_buf), SEEK_CUR)
    ) {
        throw new CompresseException("", "failed to lseek src_fd");
    };
    while(true) {
        ssize_t read_ret = read(src_fd_, read_buf, MAX_BUF_SIZE);
        if(0 == read_ret) {
            break;
        }
        if(-1 == read_ret) {
            throw new CompresseException("", "read failed on " + src_file_);
        }
        for(ssize_t i = 0; i<read_ret; i++) {
            const string &code = coding_table_[read_buf[i]];
            for(const unsigned char c : code) {
                int set_bit = 0;
                if('1'==c) {
                    set_bit = 1;
                }
                byte_buf = Utils::SetBit(byte_buf, bit_index, set_bit);
                bit_index = (bit_index+1)%8;
                if(0 == bit_index) {
                    write_buf[write_buf_index] = byte_buf;
                    write_buf_index = (write_buf_index+1)%MAX_BUF_SIZE;
                    if(0 == write_buf_index) {
                        if(MAX_BUF_SIZE != write(dst_fd_, write_buf, MAX_BUF_SIZE)) {
                            throw new CompresseException("", "write failed on " + dst_file_);
                        }
                    }
                    byte_buf = 0; // 方便后面处理，无需进行末尾补零
                }
            }
        }
    }
    if(write_buf_index != write(dst_fd_, write_buf, write_buf_index)) {
        throw new CompresseException("", "write failed on " + dst_file_);
    }
    if(0!=last_byte_effective_) {
        if( sizeof(byte_buf) != pwrite(dst_fd_, &byte_buf, sizeof(byte_buf), last_byte_pos)) {
            throw new CompresseException("", "pwrite failed on " + dst_file_);
        }
    }
}

void Compresser::Clear() {
    if(-1 == close(src_fd_)) {
        throw new CompresseException("", "failed to close src_fd");
    }

    if(-1 == close(dst_fd_)) {
        throw new CompresseException("", "failed to close dst_fd");
    }

    // TODO
    // refactor
    remove(src_file_.c_str());
}

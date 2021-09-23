#include "compresser.h"
#include "definition.h"

using namespace std;

Compresser &Compresser::GetInstance() {
    static Compresser c;
    return c;
}

/** 
 * 压缩文件命名：xxx.cprs
 * 
 * 1、srcfile检查
 * 2、词频统计，得到词频数组（数组存储哈夫曼树指针）
 * 3、构建哈夫曼树，得到字符编码表（ 字符-编码）
 * 4、写入文件头：压缩文件标识（可选）、字符频度表、最后一个字节有效位数
 * 5、逐字符进行写入
 * 6、清理工作
 */
int Compresser::Compress(const std::string &src_file) {
    try
    {
        Init(src_file);
        WordFreqCount();
        GenerateCodingTable();
        WriteHeader();

        // 5、逐字符进行写入

        // 6、清理工作
        Clear();
    }
    catch(const CompresseException& e)
    {
        errs_.push_back(e);
    }

    return 0;
}

/**
 * 压缩文件名称：例如src名为a.backup，则dst命名为a.backup.cmps
 */
void Compresser::Init(const std::string &src_file) {
    // 检查src_file
    if(src_file.empty()) {
        throw CompresseException("bad path");
    }
    struct stat st_buf;
    if(-1 == lstat(src_file.c_str(), &st_buf)) {
        if(errno == ENOENT) { // no entry
            throw CompresseException("no such file or directory: " + src_file);
        } else {
            throw CompresseException("lstat failed on " + src_file);
        }
    }
    if(!S_ISREG(st_buf.st_mode)) {
        throw CompresseException("not a regular file: " + src_file);
    }
    src_file_ = src_file;

    // 打开src_file
    src_fd_ = open(src_file_.c_str(), O_RDONLY);
    if(-1 == src_fd_) {
        throw CompresseException("open failed on " + src_file_);
    }

    // 成员变量init
    dst_file_ = src_file_ + ".cmps";
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
            throw CompresseException("read failed on " + src_file_);
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
        throw CompresseException("fail to build huffman tree");
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
 * Todo: 统计末尾有效位数
 * 压缩文件header从简：词频个数 + 末尾byte有效位数 + 字符及词频
 */
void Compresser::WriteHeader() {
    dst_fd_ = open(dst_file_.c_str(), O_WRONLY | O_CREAT, 0777);
    if(-1 == dst_fd_) {
        throw CompresseException("open failed on " + dst_file_);
    }
    unsigned int word_count = 0;
    for(TreeNode *t : counts_) {
        if(nullptr!=t) word_count++;
    }
    if(sizeof(word_count) != write(dst_fd_, &word_count, sizeof(word_count))) {
        throw CompresseException("write failed on " + dst_file_); 
    }

    // 计算末尾byte有效位数
    

    for(TreeNode *t : counts_) {
        if(nullptr!=t) {
            if(
                sizeof(t->ch_)!= write(dst_fd_, &t->ch_, sizeof(t->ch_)) || 
                sizeof(t->weight_)!=write(dst_fd_, &t->weight_, sizeof(t->weight_))
            ) {
                throw CompresseException("write failed on " + dst_file_); 
            }
        }
    }
}

void Compresser::WriteData() {
    // 对src_fd进行lseek
    if(-1 == lseek(src_fd_, 0, SEEK_SET)) {
        throw CompresseException("fail to lseek src_fd");
    }
    unsigned char read_buf[MAX_BUF_SIZE];
    unsigned char write_buf[MAX_BUF_SIZE];
    unsigned char byte_buf;
    int bit_index = 0;
    while(true) {
        ssize_t read_ret = read(src_fd_, read_buf, MAX_BUF_SIZE);
        if(0 == read_ret) {
            break;
        }
        if(-1 == read_ret) {
            throw CompresseException("read failed on " + src_file_);
        }
        for(ssize_t i = 0; i<read_ret; i++) {
            string code = coding_table_[read_buf[i]];
        }
    }
}

void Compresser::Clear() {
    if(-1 == close(src_fd_)) {
        throw CompresseException("fail to close src_fd");
    }

    // if(-1 == close(dst_fd_)) {
    //     throw CompresseException("fail to close dst_fd");
    // }
}

/** 
 * 本哈夫曼树以byte词频为基础
 * 要求counts长度大于1
 */
TreeNode *TreeNode::BuildHuffman(const vector<TreeNode *> &counts) {
    // 将基础结点加入小顶堆
    function<bool(const TreeNode *x, const TreeNode *y)> cmp(TreeNode::Cmp);
    priority_queue<TreeNode *, vector<TreeNode *>, 
                   function<bool(const TreeNode *x, const TreeNode *y)>> my_heap(cmp);
    for(TreeNode *node : counts) {
        if(nullptr!=node) {
            my_heap.push(node);
        }
    }
    // 哈夫曼核心算法：构建树，并且进行叶节点编码
    if(my_heap.empty() || my_heap.size()==1) {
        return nullptr;
    }
    while(my_heap.size()>1) {
        TreeNode *left_node = my_heap.top();
        my_heap.pop();
        TreeNode *right_node = my_heap.top();
        my_heap.pop();
        TreeNode *root = new TreeNode;
        root->weight_ = left_node->weight_ + right_node->weight_;
        root->left_ = left_node;
        root->right_ = right_node;
        my_heap.push(root);
    }
    string code{""};
    GenerateCoding(my_heap.top(), code);
    return my_heap.top();
}

void TreeNode::GenerateCoding(TreeNode *root, string &code) {
    if(nullptr == root) return;
    if(nullptr==root->left_ && nullptr==root->right_) {
        root->code_=code;
    }
    code.push_back('0');
    GenerateCoding(root->left_, code);
    code.pop_back();
    code.push_back('1');
    GenerateCoding(root->right_, code);
    code.pop_back();
}
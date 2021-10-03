#include "decrypt.h"

using namespace std;

int Decryptor::Handle(const std::string &src, const std::string &dst) {
    try
    {
        Init(src, dst);
        CheckDigest();
        Decrypt();
        Clear();
    }
    catch(BaseException *err)
    {
        errs_.push_back(shared_ptr<BaseException>(err));
    }
    
    // TODO
    // Exception handle
    int ret = ExceptionContainer::ShowErrs();

    return ret;
}

void Decryptor::Init(const std::string &src, const std::string &dst) {
    // 检查src_file
    if(src.empty()) {
        throw new DecryptException("", "bad path");
    }
    struct stat st_buf;
    if(-1 == lstat(src.c_str(), &st_buf)) {
        throw new DecryptException("", "lstat failed on " + src);
    }
    if(!S_ISREG(st_buf.st_mode)) {
        throw new DecryptException("", "not a regular file: " + src);
    }
    src_file_ = src;
    // 打开src_file
    src_fd_ = open(src_file_.c_str(), O_RDONLY);
    if(-1 == src_fd_) {
        throw new DecryptException("", "open failed on " + src_file_);
    }

    // 打开dst_file
    dst_file_ = dst;
    dst_fd_ = open(dst_file_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(-1 == dst_fd_) {
        throw new DecryptException("", "open failed on dst file");
    }

    // 成员变量init
    errs_.clear();
    if("" == pwd_) {
        throw new DecryptException("", "bad password");
    }
}

void Decryptor::CheckDigest() {
    unsigned char digest[20];
    if(16 != read(src_fd_, digest, 16)) {
        throw new DecryptException("", "failed to read " + src_file_);
    }
    unsigned char out[20];
    MD5((const unsigned char *)pwd_.c_str(), pwd_.size(), out);
    for(int i=0; i<16; i++) {
        if(digest[i]!=out[i]) {
            throw new DecryptException("", "wrong password");
        }
    }
}

void Decryptor::Decrypt() {
    // 先得到加密前文件的size
    if(sizeof(src_file_size_) != 
        read(src_fd_, &src_file_size_, sizeof(src_file_size_))) {
        throw new DecryptException("", "failed to read " + src_file_);
    }
    // 生成密钥
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0, AES_BLOCK_SIZE);
    AES_KEY my_key;
    string pwd_padding(16, '\0');
    int loop_times = min(16, (int)pwd_.size());
    for(int i=0; i<loop_times; i++) {
        pwd_padding[i] = pwd_[i];
    }
    if(0 > AES_set_decrypt_key((const unsigned char *)pwd_padding.c_str(), 
        128, &my_key)) {
        throw new DecryptException("", "failed to generate AES key");
    }
    // 数据解密
    unsigned char read_buf[MAX_BUF_SIZE];
    unsigned char write_buf[MAX_BUF_SIZE];
    off_t full_block_num = src_file_size_/MAX_BUF_SIZE;
    off_t last_block_bytenum = src_file_size_%MAX_BUF_SIZE;
    for(off_t i=0; i<full_block_num; i++) {
        ssize_t buf_num = read(src_fd_, read_buf, MAX_BUF_SIZE);
        if(0 == buf_num) {
            break;
        }
        if(-1 == buf_num || MAX_BUF_SIZE != buf_num) {
            throw new DecryptException("", "read failed on " + src_file_);
        }
        AES_cbc_encrypt(read_buf, write_buf, MAX_BUF_SIZE, &my_key, iv, AES_DECRYPT);
        if(MAX_BUF_SIZE != write(dst_fd_, write_buf, MAX_BUF_SIZE)) {
            throw new DecryptException("", "failed to write dst file");
        }
    }
    if(0!=last_block_bytenum) {
        size_t last_block_encrypted = last_block_bytenum;
        while(0!=last_block_encrypted%AES_BLOCK_SIZE) {
            last_block_encrypted++;
        }
        if(last_block_encrypted != read(src_fd_, read_buf, MAX_BUF_SIZE)) {
            // 源文件长度有问题
            throw new DecryptException("", "something is wrong with src file size");
        }
        AES_cbc_encrypt(read_buf, write_buf, last_block_encrypted, &my_key, iv, AES_DECRYPT);
        if(last_block_bytenum != write(dst_fd_, write_buf, last_block_bytenum)) {
            throw new DecryptException("", "failed to write dst file");
        }
    }
}

void Decryptor::Clear() {
    pwd_="";
    if(-1 == close(src_fd_)) {
        throw new DecryptException("", "failed to close src_fd");
    }
    fsync(dst_fd_);
    if(-1 == close(dst_fd_)) {
        throw new DecryptException("", "failed to close dst_fd");
    }
}
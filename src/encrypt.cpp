#include "encrypt.h"

#include <iostream>

using namespace std;

int Encryptor::Handle(const std::string &src, const std::string &dst) {
    int err_code = 0;
    try
    {
        Init(src, dst);
        GenerateDigest();
        Encrypt();
        Clear();
    }
    catch(BaseException *err)
    {
        errs_.push_back(shared_ptr<BaseException>(err));
    }

    // TODO
    // refactor
    err_code = BaseHandler::Handle(dst_file_, dst);
    if (!IsEnd()){
        remove(dst_file_.c_str());
    }
    // exception handle
    err_code |= ExceptionContainer::ShowErrs();

    return err_code;
}

void Encryptor::Init(const std::string &src, const std::string &dst) {
    // 检查src_file
    if(src.empty()) {
        throw new EncryptException("", "bad path");
    }
    struct stat st_buf;
    if(-1 == lstat(src.c_str(), &st_buf)) {
        throw new EncryptException("", "lstat failed on " + src);
    }
    if(!S_ISREG(st_buf.st_mode)) {
        throw new EncryptException("", "not a regular file: " + src);
    }
    src_file_ = src;
    src_file_size_ = st_buf.st_size;

    // 打开src_file
    src_fd_ = open(src_file_.c_str(), O_RDONLY);
    if(-1 == src_fd_) {
        throw new EncryptException("", "open failed on " + src_file_);
    }

    // 打开dst_file
    // TODO
    // refactor
    if (IsEnd()){
        dst_file_ = dst+"/"+Utils::ReNameBase(src, "enc");
    } else{
        dst_file_ = "/tmp/"+Utils::ReNameBase(src, "enc");
    }
    dst_fd_ = open(dst_file_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(-1 == dst_fd_) {
        throw new EncryptException("", "failed to open dst file");
    }

    // 成员变量init
    errs_.clear();
    if("" == pwd_) {
        throw new EncryptException("", "bad password");
    }
}

void Encryptor::GenerateDigest() {
    unsigned char out[20];
    MD5((const unsigned char *)pwd_.c_str(), pwd_.size(), out);
    if(16 != write(dst_fd_, out, 16)) {
        throw new EncryptException("", "failed to write dst_fd");
    }
}

/**
 * 加密文件结构：digest + 文件大小 + 加密数据
 * 注意：数据末尾块，不够16字节时，补0
 */
void Encryptor::Encrypt() {
    // 因AES加密特性，需先写入文件长度
    if(sizeof(src_file_size_) != 
        write(dst_fd_, &src_file_size_, sizeof(src_file_size_))) {
        throw new EncryptException("", "failed to write dst_fd");
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
    if(0 > AES_set_encrypt_key((const unsigned char *)pwd_padding.c_str(), 
        128, &my_key)) {
        throw new EncryptException("", "failed to generate AES key");
    }
    // 数据加密
    unsigned char read_buf[MAX_BUF_SIZE];
    unsigned char write_buf[MAX_BUF_SIZE];
    while (true) {
        ssize_t buf_num = read(src_fd_, read_buf, MAX_BUF_SIZE);
        if(0 == buf_num) {
            break;
        }
        if(-1 == buf_num) {
            throw new EncryptException("", "read failed on " + src_file_);
        }
        while(0!=buf_num%AES_BLOCK_SIZE) {
            // 最后一次需特殊处理
            read_buf[buf_num++] = '\0';
        }
        AES_cbc_encrypt(read_buf, write_buf, buf_num, &my_key, iv, AES_ENCRYPT);
        if(buf_num != write(dst_fd_, write_buf, buf_num)) {
            throw new EncryptException("", "failed to write dst file");
        }
    }
}

void Encryptor::Clear() {
    pwd_="";
    if(-1 == close(src_fd_)) {
        throw new EncryptException("", "failed to close src_fd");
    }
    fsync(dst_fd_);
    if(-1 == close(dst_fd_)) {
        throw new EncryptException("", "failed to close dst_fd");
    }
}
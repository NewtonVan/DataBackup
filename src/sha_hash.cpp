#include "sha_hash.h"

using namespace std;

int Sha256Hasher::Handle(const std::string &src, const std::string &dst) {
    try
    {
        Init(dst);
        Hash(src);
        Clear();
    }
    catch(const BaseException &err)
    {
        errs_.push_back(err);
    }
    
    // TODO
    // Exception handle
    for (Exception &err : errs_){
        err.what();
    }

    return 0;
}

void Sha256Hasher::Init(const std::string &dst) {
    // 打开dst file
    dst_file_ = dst;
    dst_fd_ = open(dst_file_.c_str(), O_WRONLY | O_CREAT, 0777);
    if(-1 == dst_fd_) {
        throw ShaException("", "open failed on dst file");
    }

    errs_.clear();
}

void Sha256Hasher::Hash(const std::string &src) {
    struct stat st_buf;
    if(-1 == lstat(src.c_str(), &st_buf)) {
        throw ShaException("", "lstat failed on " + src);
    }
    if(S_ISDIR(st_buf.st_mode)) {
        HashDir(src, st_buf);
    } else if(S_ISREG(st_buf.st_mode)) {
        HashReg(src, st_buf);
    } else if(S_ISFIFO(st_buf.st_mode)) {
        HashFifo(src, st_buf);
    } else if(S_ISLNK(st_buf.st_mode)) {
        HashLink(src, st_buf);
    } else {
        return;
    }
}

void Sha256Hasher::HashDir(const std::string &src, const struct stat &st_buf) {
    string line_write = string(basename(src.c_str())) + ' ' + "DIR" + '\n';
    WriteLine(line_write);
    // 目录项递归
    DIR *dirp;
    dirent *dp;
    dirp = opendir(src.c_str());
    if(NULL == dirp) {
        throw ShaException("", "opendir failed on " + src);
    }
    for(;;) {
        errno = 0;
        dp = readdir(dirp);
        if(dp==NULL) {
            break;
        }
        const string d_name(dp->d_name);
        if(d_name=="." || d_name=="..") {
            continue;
        }
        // 关键递归
        const string child_filename = src+'/'+d_name;
        Hash(child_filename);
    }
    if(0 != errno) {
        throw ShaException("", "readdir failed on " + src);
    }
    // close dir fd
    if(-1 == closedir(dirp)) {
        throw ShaException("", "closedir failed on " + src);
    }
    // 还原访问时间
    RestoreAccessTime(src, st_buf);
}

void Sha256Hasher::HashReg(const std::string &src, const struct stat &st_buf) {
    char hash_val[65];
    sha256_file(src.c_str(), hash_val);
    string line_write = string(basename(src.c_str())) + ' ' + string(hash_val) + '\n';
    WriteLine(line_write);
    RestoreAccessTime(src, st_buf);
}

void Sha256Hasher::HashFifo(const std::string &src, const struct stat &st_buf) {
    string line_write = string(basename(src.c_str())) + ' ' + "FIFO" + '\n';
    WriteLine(line_write);
}

void Sha256Hasher::HashLink(const std::string &src, const struct stat &st_buf) {
    char buf[PATH_MAX+1];
    ssize_t read_num = readlink(src.c_str(), buf, PATH_MAX);
    if(-1==read_num) {
        throw ShaException("", "readlink failed on " + src);
    }
    buf[read_num] = '\0';
    char hash_val[65];
    sha256_string(buf, hash_val);

    string line_write = string(basename(src.c_str())) + ' ' + string(hash_val) + '\n';
    WriteLine(line_write);
    RestoreAccessTime(src, st_buf);
}

void Sha256Hasher::WriteLine(const std::string &line_write) {
    if(line_write.size() != write(dst_fd_, line_write.c_str(), line_write.size())) {
        throw ShaException("", "write failed on dst file");
    }
}

void Sha256Hasher::Clear() {
    if(-1 == close(dst_fd_)) {
        throw ShaException("", "failed to close dst file");
    }
}

void Sha256Hasher::RestoreAccessTime(const std::string &src_file, const struct stat &st_buf) {
    timespec tm[2];
    tm[0] = st_buf.st_atim;
    tm[1].tv_nsec = UTIME_OMIT;
    if(utimensat(AT_FDCWD, src_file.c_str(), tm, AT_SYMLINK_NOFOLLOW)==-1) {
        throw ShaException("", "utimenstat failed on " + src_file);
    }
}

void Sha256Hasher::sha256_hash_string (char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65]) {
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

void Sha256Hasher::sha256_string(const char *str, char outputBuffer[65]) {
    char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    if(1 != SHA256_Init(&sha256)) {
        throw ShaException("", "SHA256_Init failed");
    }
    if(1 != SHA256_Update(&sha256, str, strlen(str))) {
        throw ShaException("", "SHA256_Init failed");
    }
    if(1 != SHA256_Final((unsigned char *)hash, &sha256)) {
        throw ShaException("", "SHA256_Init failed");
    }
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

void Sha256Hasher::sha256_file(const char *path, char outputBuffer[65]) {
    FILE *file = fopen(path, "rb");
    if(!file) {
        throw ShaException("", "fopen failed on " + std::string(path));
    }

    char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    if(1 != SHA256_Init(&sha256)) {
        throw ShaException("", "SHA256_Init failed");
    }
    const int bufSize = 32768;
    char *buffer = (char *)malloc(bufSize);
    if(!buffer) {
        throw ShaException("", "malloc failed");
    }
    int bytesRead = 0;
    while((bytesRead = fread(buffer, 1, bufSize, file))) {
        if(1 != SHA256_Update(&sha256, buffer, bytesRead)) {
            throw ShaException("", "SHA256_Update failed");
        }
    }
    if(1 != SHA256_Final((unsigned char *)hash, &sha256)) {
        throw ShaException("", "SHA256_Final failed");
    }
    sha256_hash_string(hash, outputBuffer);
    if(0!=fclose(file)) {
        throw ShaException("", "fclose failed on " + std::string(path));
    }
    free(buffer);
}
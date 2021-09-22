#include "header.h"

Header &Header::GetInstance()
{
    static Header h;
    return h;
}

int Header::Serialize(int backup_fd) const
{
    if(backup_fd<0) {
        return -1;
    }

    if(
        write(backup_fd, &file_path_len_, sizeof(file_path_len_)) != sizeof(file_path_len_) || 
        write(backup_fd, &ln_path_len_, sizeof(ln_path_len_)) != sizeof(ln_path_len_) || 
        write(backup_fd, &st_ino_, sizeof(st_ino_)) != sizeof(st_ino_) || 
        write(backup_fd, &st_mode_, sizeof(st_mode_)) != sizeof(st_mode_) || 
        write(backup_fd, &st_nlink_, sizeof(st_nlink_)) != sizeof(st_nlink_) || 
        write(backup_fd, &st_uid_, sizeof(st_uid_)) != sizeof(st_uid_) || 
        write(backup_fd, &st_gid_, sizeof(st_gid_)) != sizeof(st_gid_) || 
        write(backup_fd, &st_atime_, sizeof(st_atime_)) != sizeof(st_atime_) || 
        write(backup_fd, &st_mtime_, sizeof(st_mtime_)) != sizeof(st_mtime_) || 
        write(backup_fd, &block_num_, sizeof(block_num_)) != sizeof(block_num_) || 
        write(backup_fd, &padding_, sizeof(padding_)) != sizeof(padding_) || 
        write(backup_fd, file_path_.c_str(), file_path_len_) != file_path_len_ || 
        write(backup_fd, ln_path_.c_str(), ln_path_len_) != ln_path_len_
    ) {
        return -1;
    }

    return 0;
}

int Header::DeSerialize(int backup_fd)
{
    if (backup_fd < 0){
        return -1;
    }

    // TODO
    // using lseek may have wired result when the file is to big
    // from man page, it will return (off_t)-1 and errno is set
    // Actually, I highly recommand the whole project should use FILE* instead of
    // naive fd which makes the code less robust
    off_t old_pos = lseek(backup_fd, 0, SEEK_CUR);
    off_t end_pos = lseek(backup_fd, 0, SEEK_END);
    if (old_pos == end_pos){
        return 1;
    }
    lseek(backup_fd, old_pos, SEEK_SET);

    // TODO
    // c_str may have some problem, wait to be checked
    if (
        read(backup_fd, &file_path_len_, sizeof(file_path_len_)) != sizeof(file_path_len_) || 
        read(backup_fd, &ln_path_len_, sizeof(ln_path_len_)) != sizeof(ln_path_len_) || 
        read(backup_fd, &st_ino_, sizeof(st_ino_)) != sizeof(st_ino_) || 
        read(backup_fd, &st_mode_, sizeof(st_mode_)) != sizeof(st_mode_) || 
        read(backup_fd, &st_nlink_, sizeof(st_nlink_)) != sizeof(st_nlink_) || 
        read(backup_fd, &st_uid_, sizeof(st_uid_)) != sizeof(st_uid_) || 
        read(backup_fd, &st_gid_, sizeof(st_gid_)) != sizeof(st_gid_) || 
        read(backup_fd, &st_atime_, sizeof(st_atime_)) != sizeof(st_atime_) || 
        read(backup_fd, &st_mtime_, sizeof(st_mtime_)) != sizeof(st_mtime_) || 
        read(backup_fd, &block_num_, sizeof(block_num_)) != sizeof(block_num_) || 
        read(backup_fd, &padding_, sizeof(padding_)) != sizeof(padding_) || 
        read(backup_fd, file_path_.c_str(), file_path_len_) != file_path_len_ || 
        read(backup_fd, ln_path_.c_str(), ln_path_len_) != ln_path_len_
    ){
        return -1;
    }

    return 0;
}

std::string Header::getFilePath()
{
    return file_path_;
}

std::string Header::getSymbol()
{
    return ln_path_;
}

size_t Header::getLenFilePath()
{
    return file_path_len_;
}

size_t Header::getLenSymbol()
{
    return ln_path_len_;
}

ino_t Header::getIno()
{
    return st_ino_;
}

mode_t Header::getMode()
{
    return st_mode_;
}

nlink_t Header::getNumLink()
{
    return st_nlink_;
}

uid_t Header::getUid()
{
    return st_uid_;
}

gid_t Header::getGid()
{
    return st_gid_;
}

timespec Header::getAccessTime()
{
    return st_atime_;
}

timespec Header::getModifyTime()
{
    return st_mtime_;
}

ulong Header::getNumBlock()
{
    return block_num_;
}

uint Header::getPadding()
{
    return padding_;
}

void Header::setFilePath(const std::string &file_path)
{
    this->file_path_ = file_path;
}

void Header::setSymbol(const std::string &ln_path)
{
    this->ln_path_ = ln_path;
}

void Header::setLenFilePath(const size_t file_path_len)
{
    this->file_path_len_ = file_path_len;
}

void Header::setLenSymbol(const size_t ln_path_len)
{
    this->ln_path_len_ = ln_path_len;
}

void Header::setIno(const ino_t st_ino)
{
    this->st_ino_ = st_ino;
}

void Header::setMode(const mode_t st_mode)
{
    this->st_mode_ = st_mode;
}

void Header::setNumLink(const nlink_t st_nlink)
{
    this->st_nlink_ = st_nlink;
}

void Header::setUid(const uid_t st_uid)
{
    this->st_uid_ = st_uid;
}

void Header::setGid(const gid_t st_gid)
{
    this->st_gid_ = st_gid;
}

void Header::setAccessTime(const timespec access_time)
{
    this->st_atime_ = access_time;
}

void Header::setModifyTime(const timespec modify_time)
{
    this->st_mtime_ = modify_time;
}

void Header::setNumBlock(const ulong block_num)
{
    this->block_num_ = block_num;
}

void Header::setPadding(const uint padding)
{
    this->padding_ = padding;
}
#include "header.h"

Header::Header()
{
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
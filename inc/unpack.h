#ifndef UNPACK_H
#define UNPACK_H

#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <map>

#include "header.h"

// TODO
// It's a better design use interface-implementation structure
// the "source" of all the exception might as well defined in "header.h"
// packer, unpacker, copier should inherit from it
class UnPackException{
private:
    std::string file_nm_;
    std::string msg_;
public:
    UnPackException(const std::string &file_nm, const std::string &msg)
        : file_nm_(file_nm), msg_(msg)
    {

    }
    std::string ErrMsg()
    {
        return msg_;
    }
    std::string FileName()
    {
        return file_nm_;
    }
};
class UnPacker{
public:
    static UnPacker& GetInstance();
    void UnPack(const std::string src, const std::string dst);
private:
    UnPacker() = default;
    ~UnPacker() = default;
    UnPacker(const UnPacker &up) = delete;
    const UnPacker& operator = (const UnPacker &up) = delete;
    void Extract();
    void UnPackDir();
    void UnPackReg();
    void UnPackFIFO();
    void UnPackSLNK();
    void Init(const std::string &src, const std::string &dst);
    void Clear(const std::string &src);
    // TODO
    // It's proper depart if from unpacker and use it as a common tools
    void RecurMkdir(const std::string &dst);

    std::string abs_parent_path_;
    std::string dst_file_;
    int fd_backup_;
    std::map<ino_t, std::string> hard_lk_map_;
    std::vector<UnPackException> errs_;
};

#endif
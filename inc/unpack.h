#ifndef UNPACK_H
#define UNPACK_H

#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <algorithm>

#include "header.h"
#include "handler.h"
#include "exception_interface.h"

// TODO
// It's a better design use interface-implementation structure
// the "source" of all the exception might as well defined in "header.h"
// packer, unpacker, copier should inherit from it
class UnPackException : public BaseException{
public:
    UnPackException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg)
    {

    }
    std::string what()
    {
        return "Unpack Exception : "+BaseException::what();
    }
};
// TODO
// Reconstruct the class
// ChainOfResponsibility is a better choice than Singleton
class UnPacker : public BaseHandler{
public:
    UnPacker() = default;
    // static UnPacker& GetInstance();
    int Handle(const std::string &src, const std::string &dst) override;
private:
    ~UnPacker() = default;
    // UnPacker(const UnPacker &up) = delete;
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
    std::vector<Exception> errs_;
    Header header_;
};

#endif
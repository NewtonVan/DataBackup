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
#include "utils.h"

class UnPackException : public BaseException{
public:
    UnPackException(const std::string &file_nm, const std::string &msg)
        : BaseException(file_nm, msg)
    {

    }
    std::string what() const
    {
        return "Unpack Exception : "+BaseException::what();
    }
};

class UnPacker : public BaseHandler, public ExceptionContainer{
public:
    UnPacker() = default;
    int Handle(const std::string &src, const std::string &dst) override;
private:
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
    Header header_;
};

#endif
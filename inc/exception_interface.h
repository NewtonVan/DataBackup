#ifndef EXCEPTION_INTERFACE_H
#define EXCEPTION_INTERFACE_H

#include <string>
#include <memory>

using std::shared_ptr;

class Exception{
public:
    virtual std::string what() = 0;
};

class BaseException : public Exception{
private:
    std::string file_nm_;
    std::string msg_;
public:
     BaseException(const std::string &file_nm, const std::string &msg)
        : file_nm_(file_nm), msg_(msg)
    {

    }
    std::string what()
    {
        return file_nm_ + " : " +msg_;
    }
};
#endif
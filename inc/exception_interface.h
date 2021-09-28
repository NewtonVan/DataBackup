#ifndef EXCEPTION_INTERFACE_H
#define EXCEPTION_INTERFACE_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>

using std::shared_ptr;

class Exception{
public:
    virtual std::string what() const = 0;
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
    std::string what() const
    {
        return file_nm_ + " : " +msg_;
    }
};

class ExceptionContainer{
protected:
    std::vector<shared_ptr<BaseException> > errs_;
public:
    int ShowErrs()
    {
        if (errs_.empty()){
            return 0;
        }

        for (auto err : errs_){
            std::cerr << err->what() << std::endl;
        }

        return -1;
    }
};

#endif
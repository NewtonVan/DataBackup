#ifndef HANDLER_H
#define HANDLER_H

#include <string>

class Handler{
public:
    virtual Handler *SetNext(Handler *handler) = 0;
    virtual int Handle(const std::string &src, const std::string &dst) = 0;
};

class BaseHandler : public Handler{
private:
    Handler *next_handler_;
public:
    BaseHandler() : next_handler_(nullptr)
    {

    }
    Handler *SetNext(Handler *handler) override
    {
        this->next_handler_ = handler;
        return handler;
    }
    int Handle(const std::string &src, const std::string &dst) override
    {
        if (this->next_handler_){
            return this->next_handler_->Handle(src, dst);
        }

        return -1;
    }
};

#endif
#ifndef ERROR_MESSAGER_H_
#define ERROR_MESSAGER_H_

#include <iostream>
#include <string>

class ErrorMessager {
public:
    static void PrintError(std::ostream &os, const std::string &msg, int the_errno = 0) {
        os << msg;
        if(the_errno!=0) {
            os << ", errno: " << the_errno;
        }
        os << std::endl;
    }
private:
    ErrorMessager() = default;
};

#endif
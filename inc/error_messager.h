#ifndef ERROR_MESSAGER_H_
#define ERROR_MESSAGER_H_

#include <iostream>
#include <string>

class ErrorMessager {
public:
    static void PrintError(std::ostream &os, const std::string &msg) {
        os << msg << std::endl;
    }
private:
    ErrorMessager() = default;
};

#endif
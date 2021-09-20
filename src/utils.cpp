#include "utils.h"

using namespace std;

const std::string Utils::GetFileNameOnLinux(const string &file_path) {
    // linux下
    char ch = '/';
    const char *q = strrchr(file_path.c_str(), ch) + 1;

    return string(q);
}
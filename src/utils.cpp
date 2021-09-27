#include "utils.h"

using namespace std;

const std::string Utils::BaseName(const string &file_path) {
    // linux下
    char ch = '/';
    const char *q = strrchr(file_path.c_str(), ch) + 1;

    return string(q);
}

unsigned char Utils::SetBit(unsigned char uc, int bit_index, int set_bit) {
    if(bit_index<0 || bit_index>7) return '\0';
    if(set_bit) { // 置1
        unsigned char mask = 1 << (7-bit_index);
        return uc | mask;
    } else {
        unsigned char mask = ~(1 << (7-bit_index));
        return uc & mask;
    }
    
}

int Utils::GetBit(unsigned char uc, int bit_index) {
    if(bit_index<0 || bit_index>7) return -1;
    return (uc & (1 << (7-bit_index))) ? 1 : 0;
}
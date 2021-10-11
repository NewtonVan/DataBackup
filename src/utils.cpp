#include "utils.h"

using namespace std;

const std::string Utils::BaseName(const string &file_path) {
    // linux下
    char ch = '/';
    const char *q = strrchr(file_path.c_str(), ch) + 1;

    return string(q);
}

const std::string Utils::ReNameBase(const std::string &file_path, const std::string &surfix) {
    string base_name = BaseName(file_path);
    char ch = '.';
    return string(base_name.begin(), base_name.begin()+base_name.find_last_of(ch))+ch+surfix;
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

void Utils::Copy(Header &h, int fd_src, int fd_dst, int padding_on)
{
    char buf[CP_BLOCK_SIZE+3];
    ssize_t buf_lth;
    ulong block_num = h.getNumBlock();

    for (ulong i = 0; i < block_num; ++i){
        buf_lth = read(fd_src, buf, CP_BLOCK_SIZE);
        if (-1 == buf_lth){
            throw CopyException(h.getFilePath(), "Error reading when Copying");
        }

        if (block_num-1 == i){
            if (padding_on){
                memset(buf+buf_lth, 0, h.getPadding());
                buf_lth = CP_BLOCK_SIZE;
            } else{
                buf_lth -= h.getPadding();
            }
        }

        if (write(fd_dst, buf, buf_lth) != buf_lth){
            throw CopyException(h.getFilePath(), "Error writing when Copying");
        }
    }
}

void Utils::RegPath(char *const path)
{
    int lth = strlen(path);
    if ('/' == path[lth-1]){
        return;
    }
    path[lth] = '/';
    path[++lth] = '\0';
}

void Utils::RecurMkdir(const string &dst)
{
    size_t local_offset = 0, global_offset = 0;
    // wired about string::iterator and string::_cxx11::iterator
    // string::iterator iter = dst.begin()+1;
    auto iter = dst.begin()+1;
    string token;

    while (dst.end() != iter){
        local_offset = (size_t)(find(iter, dst.end(), '/')-iter);
        if (0 == local_offset){
            throw new CopyException(dst, "Wrong format destination");
        }
        ++local_offset;
        iter += local_offset;
        global_offset += local_offset;
        token = dst.substr(0, global_offset);

        // TODO
        // only for test
        // cout<<token<<endl;

        if (0 == access(token.c_str(), F_OK)){
            struct stat st_buf;
            stat(token.c_str(), &st_buf);
            if (!S_ISDIR(st_buf.st_mode)){
                throw new CopyException(dst, "Wrong format destination");
            }
        } else{
            if (-1 == mkdir(token.c_str(), 00775)){
                throw new CopyException(dst, "Fail to create directory");
            }
        }


        if (dst.length() <= global_offset){
            break;
        }
    }
}
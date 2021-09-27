#include "backend.h"

BackEnd::BackEnd(const string &abs_cur_path)
    : abs_path_(abs_cur_path)
{

}
void BackEnd::RecurMkdir(const string &dst)
{
    size_t local_offset = 0, global_offset = 0;
    // wired about string::iterator and string::_cxx11::iterator
    // string::iterator iter = dst.begin()+1;
    auto iter = dst.begin()+1;
    string token;

    while (dst.end() != iter){
        local_offset = (size_t)(find(iter, dst.end(), '/')-iter);
        if (0 == local_offset){
            throw new UnPackException(dst, "Wrong format destination");
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
                throw new UnPackException(dst, "Wrong format destination");
            }
        } else{
            if (-1 == mkdir(token.c_str(), 00775)){
                throw new UnPackException(dst, "Fail to create directory");
            }
        }


        if (dst.length() <= global_offset){
            break;
        }
    }
}
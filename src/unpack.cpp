// TODO
// iostream include for test
#include "unpack.h"
#include "copier.h"

using namespace std;

int UnPacker::Handle(const string &src, const string &dst)
{
    // TODO
    // proper exception handle mechanism
    try
    {
        Init(src, dst);
        Extract();
        Clear(src);
    }
    catch(const BaseException &err)
    {
        errs_.push_back(err);
    }
    
    // TODO
    // Exception handle
    for (Exception &err : errs_){
        err.what();
    }

    return 0;
}

void UnPacker::Extract()
{
    while (0 == header_.DeSerialize(fd_backup_)){
        try
        {
            mode_t dst_mode = header_.getMode();
            dst_file_ = abs_parent_path_+header_.getFilePath();

            if (S_ISDIR(dst_mode)){
                UnPackDir();
            } else if (S_ISREG(dst_mode)){
                UnPackReg();
            } else if (S_ISFIFO(dst_mode)){
                UnPackFIFO();
            } else if (S_ISLNK(dst_mode)){
                UnPackSLNK();
            } else{
                throw UnPackException(dst_file_, "Undefined file type");
            }
        }
        catch(const UnPackException &err)
        {
            errs_.push_back(err);
        }
    }
}

void UnPacker::UnPackDir()
{
    if (-1 == mkdir(dst_file_.c_str(), 00775)){
        throw UnPackException(dst_file_, "Fail to create directory");
    }
}

void UnPacker::UnPackReg()
{
    // handle hard link
    // first met, memo it
    // otherwise, create a link using `link`, then skip it
    if (header_.getNumLink() > 1){
        if (hard_lk_map_.count(header_.getIno())){
            if (link(hard_lk_map_.at(header_.getIno()).c_str(), dst_file_.c_str())){
                throw UnPackException(dst_file_, "Fail to create hard link");
            }
            return;
        } else{
            hard_lk_map_.insert(make_pair(header_.getIno(), dst_file_));
        }
    }

    int fd_dst = open(dst_file_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00700);
    if (-1 == fd_dst){
        throw UnPackException(dst_file_, "Fail to create regular file");
    }

    // TODO
    // confused about cun's code, query him about variable `remain`
    Copy(header_, fd_backup_, fd_dst, 0);
    if (-1 == close(fd_dst)){
        throw UnPackException(dst_file_, "Fail to close regular file");
    }
}

void UnPacker::UnPackFIFO()
{
    // TODO
    // figure out the mode
    if (-1 == mkfifo(dst_file_.c_str(), 00664)){
        throw UnPackException(dst_file_, "Fail to create FIFO file");
    }
}

void UnPacker::UnPackSLNK()
{
    if (symlink(header_.getSymbol().c_str(), dst_file_.c_str())){
        throw UnPackException(dst_file_, "Fail to create symbolic file");
    }
}

void UnPacker::Init(const string &src, const string &dst)
{
    hard_lk_map_.clear();
    errs_.clear();
    if (0 == access(dst.c_str(), F_OK)){
        struct stat st_buf;
        stat(dst.c_str(), &st_buf);
        if (!S_ISDIR(st_buf.st_mode)){
            throw UnPackException(dst, "Target path is not a directory");
        }
    } else{
        RecurMkdir(dst);
    }

    abs_parent_path_ = dst;
    if ('/' != abs_parent_path_.back()){
        abs_parent_path_.push_back('/');
    }

    fd_backup_ = open(src.c_str(), O_RDONLY);
    if (-1 == fd_backup_){
        throw UnPackException(src, "Fail to open source backup file");
    }
}

void UnPacker::Clear(const string &src)
{
    if (-1 == close(fd_backup_)){
        throw UnPackException(src, "Fail to close source backup file");
    }
}

void UnPacker::RecurMkdir(const string &dst)
{
    size_t local_offset = 0, global_offset = 0;
    // wired about string::iterator and string::_cxx11::iterator
    // string::iterator iter = dst.begin()+1;
    auto iter = dst.begin()+1;
    string token;

    while (dst.end() != iter){
        local_offset = (size_t)(find(iter, dst.end(), '/')-iter);
        if (0 == local_offset){
            throw UnPackException(dst, "Wrong format destination");
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
                throw UnPackException(dst, "Wrong format destination");
            }
        } else{
            if (-1 == mkdir(token.c_str(), 00775)){
                throw UnPackException(dst, "Fail to create directory");
            }
        }


        if (dst.length() <= global_offset){
            break;
        }
    }
}
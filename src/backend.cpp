#include "backend.h"

void BackEnd::Handle(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    char cwdbuf[PATH_MAX + 3];
    if(NULL == getcwd(cwdbuf, PATH_MAX)) {
        // TODO
    }
    // TODO
    // need to test the correctness of RegPath
    Utils::RegPath(cwdbuf);

    BackEnd *backend = new BackEnd(string(cwdbuf));
    backend->js_parser_.Decode(msg->get_payload());

    if (
        "decrypt" == backend->js_parser_.getMethod() || 
        "uncompress" == backend->js_parser_.getMethod() || 
        "unpack" == backend->js_parser_.getMethod()
    ) {
        backend->HandleDecode();
    } else if ("copy" == backend->js_parser_.getMethod()){
        backend->HandleCopy();
    } else if ("getList" == backend->js_parser_.getMethod()){
        backend->HandleGetList();
    } else if ("getPWD"==backend->js_parser_.getMethod()){
        backend->HandleGetPWD();
    } else {
        backend->HandleEncode();
    }

    server &s = MServer::GetInstance().GetEP();
    s.send(hdl, backend->js_parser_.getJsonString(), msg->get_opcode());
}

BackEnd::BackEnd(const string &abs_cur_path)
    : err_code_(0), abs_path_(abs_cur_path)
{

}

void BackEnd::HandleEncode()
{
    unsigned char flag = 0;
    final_dst_ = "";
    src_ = js_parser_.getSrc();
    dst_ = abs_path_+js_parser_.getDst();
    Utils::RecurMkdir(dst_);
    switch (js_parser_.getMethod()[0]){
        case 'p' :
            flag= Utils::SetBit(flag, PACK_INDEX, 1);
            HandlePack();
            break;
        case 'c' :
            flag = Utils::SetBit(flag, PACK_INDEX, 1);
            flag = Utils::SetBit(flag, CMP_INDEX, 1);
            HandleCompress();
            break;
        case 'e' :
            flag = Utils::SetBit(flag, PACK_INDEX, 1);
            flag = Utils::SetBit(flag, CMP_INDEX, 1);
            flag = Utils::SetBit(flag, ENC_INDEX, 1);
            HandleEncrypt();
            break;
        default :
            err_code_ = -1;
    }

    js_parser_.Encode(err_code_, dir_entries_);

    if(!final_dst_.empty()) {
        // add end-byte
        struct stat st_buf;
        lstat(final_dst_.c_str(), &st_buf);
        int fd_dst = open(final_dst_.c_str(), O_WRONLY | O_APPEND);
        if(-1 == fd_dst) {
            // Todo
        }
        if(sizeof(flag) != write(fd_dst, &flag, sizeof(flag))) {
            // Todo
        }
        if(-1 == close(fd_dst)) {
            // Todo
        }
    }
}

void BackEnd::HandlePack()
{
    Packer *packer = new Packer;

    err_code_ = packer->Handle(src_, dst_);
    delete packer;
    final_dst_ = dst_+'/'+Utils::BaseName(src_)+".pak";
}

void BackEnd::HandleCompress()
{
    Packer *packer = new Packer;
    Compresser *compresser = new Compresser;
    packer->SetNext(compresser);

    err_code_ = packer->Handle(src_, dst_);
    delete packer;
    delete compresser;
    final_dst_ = dst_+'/'+Utils::BaseName(src_)+".cmp";
}

void BackEnd::HandleEncrypt()
{
    Packer *packer = new Packer;
    Compresser *compresser = new Compresser;
    Encryptor *encryptor = new Encryptor;
    packer->SetNext(compresser);
    compresser->SetNext(encryptor);
    encryptor->SetPassword(js_parser_.getKey());

    err_code_ = packer->Handle(src_, dst_);

    delete packer;
    delete compresser;
    delete encryptor;
    final_dst_ = dst_+'/'+Utils::BaseName(src_)+".enc";
}

void BackEnd::HandleGetList()
{
    abs_path_ = js_parser_.getPath();
    if ('/' != abs_path_.back()){
        abs_path_.push_back('/');
    }
    // cout << abs_path_ << endl;
    DIR *dirp;
    dirent *dp;
    dirp = opendir(abs_path_.c_str());
    if(NULL == dirp) {
        // TODO
        // throw PackException("", "opendir failed on " + src_file);
        err_code_ = 1;
        js_parser_.Encode(err_code_, dir_entries_);
        return;
    }
    for(;;) {
        errno = 0;
        dp = readdir(dirp);
        if(dp==NULL) {
            break;
        }
        string d_name(dp->d_name);
        if(d_name=="." || d_name=="..") {
            continue;
        }
        struct stat st_buf;
        if(-1 == lstat((abs_path_ + dp->d_name).c_str(), &st_buf)) {
            // TODO
        }
        DirEntry de;
        de.m_filename = d_name;
        if(S_ISDIR(st_buf.st_mode)) {
            de.m_filetype =  "dir";
        } else if(S_ISREG(st_buf.st_mode)) {
            de.m_filetype =  "reg";
        } else if(S_ISFIFO(st_buf.st_mode)) {
            de.m_filetype =  "fifo";
        } else if(S_ISLNK(st_buf.st_mode)) {
            de.m_filetype =  "symlink";
        } else {
            continue;
        }
        de.m_inode = st_buf.st_ino;
        de.m_size = st_buf.st_size;
        dir_entries_.push_back(de);
    }
    if(0 != errno) {
        // TODO
        // throw PackException("", "readdir failed on " + src_file);
    }
    // close dir fd
    if(-1 == closedir(dirp)) {
        // TODO
        // throw PackException("", "closedir failed on " + src_file);
    }

    js_parser_.Encode(err_code_, dir_entries_);
}

void BackEnd::HandleGetPWD()
{
    js_parser_.Encode(err_code_, abs_path_.substr(0, abs_path_.length()-1));
}

void BackEnd::HandleCopy()
{
    Packer *packer = new Packer;
    string pk_src = js_parser_.getSrc();
    string pk_dst = "/tmp";
    err_code_ = packer->Handle(pk_src, pk_dst);
    pk_dst += "/"+Utils::BaseName(pk_src)+".pak";
    delete packer;

    UnPacker *unpacker = new UnPacker;
    string cp_dst = abs_path_+js_parser_.getDst();
    err_code_ |= unpacker->Handle(pk_dst, cp_dst);
    delete unpacker;

    // TODO
    // error handle
    remove(pk_dst.c_str());

    js_parser_.Encode(err_code_, dir_entries_);
}

void BackEnd::HandleDecode()
{
    src_ = js_parser_.getSrc();
    dst_ = abs_path_ + js_parser_.getDst();

    // 1???????????????
    unsigned char flag = 0;
    int fd_src = open(src_.c_str(), O_RDONLY);
    if(-1 == fd_src) {
        // Todo
    }
    if(-1 == lseek(fd_src, -1, SEEK_END)) {
        // Todo
    }
    if(sizeof(flag)!=read(fd_src, &flag, sizeof(flag)))
    if(-1 == close(fd_src)) {
        // Todo
    }

    // 2???????????????
    struct stat st_buf;
    if(-1 == stat(src_.c_str(), &st_buf)) {
        // Todo
    }
    if(-1 == truncate(src_.c_str(), st_buf.st_size-1)) {
        // TOdo
    }

    // 3???unpack
    string t_src = src_;
    string t_dst = "";
    if (Utils::GetBit(flag, ENC_INDEX)){
        Decryptor *decryptor = new Decryptor;
        // TODO ??????
        decryptor->SetPassword(js_parser_.getKey());
        t_dst = "/tmp/"+Utils::BaseName(t_src)+".dec";
        err_code_ |= decryptor->Handle(t_src, t_dst);
        t_src = t_dst;
        delete decryptor;
    }
    if (!err_code_ && Utils::GetBit(flag, CMP_INDEX)){
        Decompresser *decomp = new Decompresser;
        t_dst = "/tmp/"+Utils::BaseName(t_src)+".dcmp";
        err_code_ |= decomp->Handle(t_src, t_dst);
        if (t_src != src_){
            // Todo
            remove(t_src.c_str());
        }
        t_src = t_dst;
        delete decomp;
    }
    if (!err_code_){
        UnPacker *unpacker = new UnPacker;
        err_code_ |= unpacker->Handle(t_src, dst_);
        if (t_src != src_){
            // TODO
            remove(t_src.c_str());
        }
        delete unpacker;
    }
    
    js_parser_.Encode(err_code_, dir_entries_);

    // 4???????????????
    fd_src = open(src_.c_str(), O_WRONLY | O_APPEND);
    if(-1 == fd_src) {
        // Todo
    }
    if(sizeof(flag) != write(fd_src, &flag, sizeof(flag))) {
        // Todo
    }
    if(-1 == close(fd_src)) {
        // Todo
    }
}
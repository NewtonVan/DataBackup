#ifndef BACKEND_H
#define BACKEND_H

#include "unpack.h"
#include "json_parser.h"
#include "m_server.h"
#include "packer.h"
#include "compresser.h"
#include "utils.h"
#include "encrypt.h"
#include "decrypt.h"
#include "decompresser.h"

#include <iostream>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

using std::cout;
using std::endl;

#define PACK_INDEX 0
#define CMP_INDEX 1
#define ENC_INDEX 2

class BackEnd{
public:
    static void Handle(websocketpp::connection_hdl hdl, server::message_ptr msg)
    {
        BackEnd *backend = new BackEnd(string("/DataBackup/DataBackup/test/target/"));
        backend->js_parser_.Decode(msg->get_payload());

        vector<DirEntry> dir_entries;
        int err_code = 0;
        // TODO
        if (
            "decrypt" == backend->js_parser_.getMethod() || 
            "uncompress" == backend->js_parser_.getMethod() || 
            "unpack" == backend->js_parser_.getMethod()
        ) {
            string src = backend->js_parser_.getSrc();
            string dst = backend->abs_path_ + backend->js_parser_.getDst();
            // 1、读取字节
            unsigned char flag = 0;
            int fd_src = open(src.c_str(), O_RDONLY);
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

            // 2、文件截断
            struct stat st_buf;
            if(-1 == stat(src.c_str(), &st_buf)) {
                // Todo
            }
            if(-1 == truncate(src.c_str(), st_buf.st_size-1)) {
                // TOdo
            }

            // 3、unpack
            string t_src = src;
            string t_dst = "";
            if (Utils::GetBit(flag, ENC_INDEX)){
                Decryptor *decryptor = new Decryptor;
                // TODO 重要
                decryptor->SetPassword("abc");
                t_dst = "/tmp/"+Utils::BaseName(t_src)+".dec";
                err_code |= decryptor->Handle(t_src, t_dst);
                t_src = t_dst;
                delete decryptor;
            }
            if (Utils::GetBit(flag, CMP_INDEX)){
                Decompresser *decomp = new Decompresser;
                t_dst = "/tmp/"+Utils::BaseName(t_src)+".dcmp";
                err_code |= decomp->Handle(t_src, t_dst);
                if (t_src != src){
                    // Todo
                    remove(t_src.c_str());
                }
                t_src = t_dst;
                delete decomp;
            }
          
            UnPacker *unpacker = new UnPacker;
            err_code |= unpacker->Handle(t_src, dst);
            if (t_src != src){
                // TODO
                remove(t_src.c_str());
            }
            delete unpacker;

            backend->js_parser_.Encode(err_code, dir_entries);

            // 4、文件还原
            fd_src = open(src.c_str(), O_WRONLY | O_APPEND);
            if(-1 == fd_src) {
                // Todo
            }
            if(sizeof(flag) != write(fd_src, &flag, sizeof(flag))) {
                // Todo
            }
            if(-1 == close(fd_src)) {
                // Todo
            }
        } else if ("copy" == backend->js_parser_.getMethod()){
            Packer *packer = new Packer;
            string pk_src = backend->js_parser_.getSrc();
            string pk_dst = "/tmp/"+Utils::BaseName(pk_src) +".pak";
            err_code = packer->Handle(pk_src, pk_dst);
            delete packer;

            UnPacker *unpacker = new UnPacker;
            string cp_dst = backend->abs_path_+backend->js_parser_.getDst();
            err_code |= unpacker->Handle(pk_dst, cp_dst);
            delete unpacker;

            // TODO
            // error handle
            remove(pk_dst.c_str());

            backend->js_parser_.Encode(err_code, dir_entries);
        } else if ("getList" == backend->js_parser_.getMethod()){
            backend->abs_path_ += backend->js_parser_.getPath() + '/';
            cout << backend->abs_path_ << endl;
            DIR *dirp;
            dirent *dp;
            dirp = opendir(backend->abs_path_.c_str());
            if(NULL == dirp) {
                // TODO
                // throw PackException("", "opendir failed on " + src_file);
                // err_code = 1;
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
                if(-1 == lstat((backend->abs_path_ + dp->d_name).c_str(), &st_buf)) {
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
                dir_entries.push_back(de);
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

            backend->js_parser_.Encode(err_code, dir_entries);
        } else{
            unsigned char flag = 0;
            string the_dst{""};
            if ("pack" == backend->js_parser_.getMethod()){
                flag= Utils::SetBit(flag, PACK_INDEX, 1);

                string pk_src = backend->js_parser_.getSrc();
                string dst_dir = backend->abs_path_+backend->js_parser_.getDst();
                backend->RecurMkdir(dst_dir);
                string pk_dst = dst_dir + '/' + Utils::BaseName(pk_src)+".pak";
                Packer *packer = new Packer;
                err_code = packer->Handle(pk_src, pk_dst);
                delete packer;
                the_dst = pk_dst;

                backend->js_parser_.Encode(err_code, dir_entries);
            } else if ("compress" == backend->js_parser_.getMethod()){
                flag = Utils::SetBit(flag, PACK_INDEX, 1);
                flag = Utils::SetBit(flag, CMP_INDEX, 1);

                // pack to tmp
                string pk_src = backend->js_parser_.getSrc();
                string pk_dst = "/tmp/"+ Utils::BaseName(pk_src) +".pak";
                Packer *packer = new Packer;
                err_code = packer->Handle(pk_src, pk_dst);
                delete packer;

                // cmp to target
                string cmp_src = pk_dst;
                string dst_dir = backend->abs_path_+backend->js_parser_.getDst();
                backend->RecurMkdir(dst_dir);
                string cmp_dst = dst_dir + '/' + Utils::BaseName(pk_src) +".cmp";
                Compresser *compresser = new Compresser;
                err_code |= compresser->Handle(cmp_src, cmp_dst);
                delete compresser;
                the_dst = cmp_dst;

                // remove tmp files
                remove(pk_dst.c_str());

                backend->js_parser_.Encode(err_code, dir_entries);
            } else if ("encrypt" == backend->js_parser_.getMethod()){
                flag = Utils::SetBit(flag, PACK_INDEX, 1);
                flag = Utils::SetBit(flag, CMP_INDEX, 1);
                flag = Utils::SetBit(flag, ENC_INDEX, 1);
                // pack to tmp
                string pk_src = backend->js_parser_.getSrc();
                string pk_dst = "/tmp/"+ Utils::BaseName(pk_src) +".pak";
                Packer *packer = new Packer;
                err_code = packer->Handle(pk_src, pk_dst);
                delete packer;

                // cmp to tmp
                string cmp_src = pk_dst;
                string cmp_dst = "/tmp/" + Utils::BaseName(pk_src) +".cmp";
                Compresser *compresser = new Compresser;
                err_code |= compresser->Handle(cmp_src, cmp_dst);
                delete compresser;

                // encrypt to target
                string enc_src = cmp_dst;
                string dst_dir = backend->abs_path_+backend->js_parser_.getDst();
                backend->RecurMkdir(dst_dir);
                string enc_dst =  dst_dir + '/' + Utils::BaseName(pk_src) +".enc";
                Encryptor *encryptor = new Encryptor;
                // Todo
                encryptor->SetPassword("abc");
                err_code |= encryptor->Handle(enc_src, enc_dst);
                delete encryptor;
                the_dst = enc_dst;
                
                // remove tmp files
                remove(pk_dst.c_str());
                remove(cmp_dst.c_str());
                
                backend->js_parser_.Encode(err_code, dir_entries);
            }
            if(!the_dst.empty()) {
                // add end-byte
                struct stat st_buf;
                lstat(the_dst.c_str(), &st_buf);
                int fd_dst = open(the_dst.c_str(), O_WRONLY | O_APPEND);
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

        server &s = MServer::GetInstance().GetEP();
        s.send(hdl, backend->js_parser_.getJsonString(), msg->get_opcode());
    }
    void Run();
    void RecurMkdir(const string &dst);
    BackEnd(const string &abs_cur_path);
private:
    JsonParser js_parser_;
    string abs_path_;   // abs_path_ end with '/'
};

#endif
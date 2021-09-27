#ifndef BACKEND_H
#define BACKEND_H

#include "unpack.h"
#include "json_parser.h"
#include "m_server.h"
#include "packer.h"
#include "compresser.h"
#include "utils.h"
#include "encrypt.h"

#include <iostream>
#include <cstring>
#include <dirent.h>
#include <errno.h>

class BackEnd{
public:
    static void Handle(websocketpp::connection_hdl hdl, server::message_ptr msg)
    {
        BackEnd *backend = new BackEnd(string("/DataBackup/test/target/"));
        backend->js_parser_.Decode(msg->get_payload());

        vector<string> vs;
        int err_code;
        unsigned char flag = 0;

        // TODO
        if ("unpack" == backend->js_parser_.getMethod()){
            UnPacker *unpacker = new UnPacker;
            err_code = unpacker->Handle(backend->js_parser_.getSrc(), backend->js_parser_.getDst());
            delete unpacker;

            backend->js_parser_.Encode(err_code, vs);
        } else if ("copy" == backend->js_parser_.getMethod()){
            Packer *packer = new Packer;
            UnPacker *unpacker = new UnPacker;

            string pk_src = backend->js_parser_.getSrc();
            string pk_dst = "/tmp/"+pk_src.substr(pk_src.rfind('/')+1, pk_src.length()) +".pak";
            err_code = packer->Handle(pk_src, pk_dst);
            
            string dst = backend->abs_path_+backend->js_parser_.getDst();
            err_code |= unpacker->Handle(pk_dst, dst);

            // TODO
            // error handle
            remove(pk_dst.c_str());
            delete unpacker;
            delete packer;

            backend->js_parser_.Encode(err_code, vs);
        } else if ("getList" == backend->js_parser_.getMethod()){
            // backend->abs_path_ = backend->js_parser_.getPath();
            
            // DIR *dirp;
            // dirent *dp;
            // dirp = opendir(backend->abs_path_.c_str());
            // if(NULL == dirp) {
            //     // throw PackException("", "opendir failed on " + src_file);
            //     err_code = 1;
            // }
            // for(;;) {
            //     errno = 0;
            //     dp = readdir(dirp);
            //     if(dp==NULL) {
            //         break;
            //     }
            //     const string d_name(dp->d_name);
            //     if(d_name=="." || d_name=="..") {
            //         continue;
            //     }
            //     if(-1==lstat)
            // }
            // if(0 != errno) {
            //     throw PackException("", "readdir failed on " + src_file);
            // }
            // // close dir fd
            // if(-1 == closedir(dirp)) {
            //     throw PackException("", "closedir failed on " + src_file);
            // }

        } else{
            if ("pack" == backend->js_parser_.getMethod()){
                flag= Utils::SetBit(flag, 0, 1);

                string pk_src = backend->js_parser_.getSrc();
                string dst_dir = backend->abs_path_+backend->js_parser_.getDst();
                backend->RecurMkdir(dst_dir);
                string pk_dst = dst_dir + '/' + Utils::BaseName(pk_src)+".pak";
                Packer *packer = new Packer;
                err_code = packer->Handle(pk_src, pk_dst);
                delete packer;

                backend->js_parser_.Encode(err_code, vs);
            } else if ("compress" == backend->js_parser_.getMethod()){
                flag = Utils::SetBit(flag, 0, 1);
                flag = Utils::SetBit(flag, 1, 1);

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

                // remove tmp files
                remove(pk_dst.c_str());

                backend->js_parser_.Encode(err_code, vs);
            } else if ("encrypt" == backend->js_parser_.getMethod()){
                flag = Utils::SetBit(flag, 0, 1);
                flag = Utils::SetBit(flag, 1, 1);
                flag = Utils::SetBit(flag, 2, 1);
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
                err_code |= encryptor->Handle(enc_src, enc_dst);
                delete encryptor;
                
                // remove tmp files
                remove(pk_dst.c_str());
                remove(cmp_dst.c_str());
                
                backend->js_parser_.Encode(err_code, vs);
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
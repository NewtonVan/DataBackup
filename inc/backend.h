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
    void Run();
    void RecurMkdir(const string &dst);
    BackEnd(const string &abs_cur_path);
    string getFinalDst();
    void setFinalDst(string final_dst);
private:
    void HandlePack();
    void HandleEncrypt();
    void HandleCompress();
    void HandleEncode();
    void HandleGetList();
    void HandleGetPWD();
    void HandleCopy();
    void HandleDecode();

private:
    int err_code_;
    JsonParser js_parser_;
    string abs_path_;   // abs_path_ end with '/'
    string final_dst_;
    string src_;
    string dst_;
    vector<DirEntry> dir_entries_;
};

#endif
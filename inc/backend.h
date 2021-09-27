#ifndef BACKEND_H
#define BACKEND_H

#include "unpack.h"
#include "json_parser.h"
#include "m_server.h"

class BackEnd{
public:
    static void Handle(websocketpp::connection_hdl hdl, server::message_ptr msg)
    {
        BackEnd *backend = new BackEnd;
        backend->js_parser_.Decode(msg->get_payload());

        vector<string> vs;
        int err_code;

        // TODO
        if ("unpack" == backend->js_parser_.getMethod()){
            UnPacker unpacker = new UnPacker;
            err_code = unpacker->Handle(backend->js_parser_.getSrc(), backend->js_parser_.getDst());
            backend->js_parser_.Encode(err_code, vs);
        } else if ("pack" == backend->js_parser_.getMethod()){
            Packer packer = new Packer;
        }

        server &s = MServer::GetInstance().GetEP();
        s.send(hdl, backend->js_parser_.getJsonString(), msg->get_opcode());
    }
    void Run();
private:
    JsonParser js_parser_;
};

#endif
#ifndef M_SERVER_H
#define M_SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::config::asio::message_type::ptr message_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class MServer{
private:
    MServer() = default;
    ~MServer() = default;
    MServer(const MServer &ms) = delete;
    const MServer& operator = (const MServer &ms) = delete;
public:
    static MServer& GetInstance();
    void Init(uint16_t port, void (*on_message_fun)(websocketpp::connection_hdl hdl, server::message_ptr msg));
    void StartServer();
    server& GetEP();
private:
    server end_point_;
};

#endif
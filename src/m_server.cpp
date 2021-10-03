#include "m_server.h"

MServer& MServer::GetInstance()
{
    static MServer s;
    return s;
}

void MServer::Init(uint16_t port, void (*on_message_fun)(websocketpp::connection_hdl hdl, server::message_ptr msg))
{
    try
    {
        end_point_.set_reuse_addr(true);
        end_point_.set_access_channels(websocketpp::log::alevel::none);
        end_point_.clear_access_channels(websocketpp::log::alevel::none);  

        end_point_.init_asio();
        end_point_.set_message_handler(on_message_fun);
        end_point_.listen(port);
        end_point_.start_accept();
    }
    catch(websocketpp::exception const &e)
    {
        // TODO
        std::cout<<e.what()<<std::endl;
    }
}

void MServer::StartServer()
{
    try
    {
        end_point_.run();
    }
    catch(websocketpp::exception const &e)
    {
        // TODO
        std::cout<<e.what()<<std::endl;
    }
    
}

server& MServer::GetEP()
{
    return end_point_;
}
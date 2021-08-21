#pragma once

#include <iostream>

#include "sqlite3.h"
#include "../../common/actor.hpp"
#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>





namespace knit
{
    namespace actor
    {
        namespace sink
        {
            namespace websocket
            {
                using source::stm32::SPIMessagePtr;
                using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::ref;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

using server = websocketpp::server<websocketpp::config::asio>;

                struct ViewParams
                {
                    std::string db;
                };
                class View : public Sink<SPIMessagePtr>
                {
                    public:
                    View(const ViewParams& params);
                    ~View();

                    private:
                    bool validate_(server &s, connection_hdl hdl);
                    void on_open(connection_hdl hdl);
                    void on_close(connection_hdl hdl);
                    void on_message(connection_hdl hdl, server::message_ptr msg);


                    ViewParams params_;
                    server server_;
                };
            }
        }
    }
}
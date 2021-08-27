#pragma once

#include <iostream>
#include <chrono>
#include <map>
#include <mutex>

#include "../../common/actor.hpp"
#include "../../source/stm32/mutex.hpp"
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
                // using websocketpp::lib::bind;
                // using websocketpp::lib::ref;
                // using websocketpp::lib::placeholders::_1;
                // using websocketpp::lib::placeholders::_2;

                using server = websocketpp::server<websocketpp::config::asio>;

                struct ViewParams
                {
                    std::vector<source::stm32::SensorName> dump_sensors;
                    std::chrono::seconds timeout;
                };
                class View : public Sink<SPIMessagePtr>
                {
                public:
                    View(const ViewParams &params);
                    ~View();

                    virtual void receive_loop(Tube<SPIMessagePtr> &in) override;
                    virtual void receive(Tube<SPIMessagePtr> &in) override;

                private:
                    void run_();
                    bool validate_(connection_hdl hdl);
                    void on_open(connection_hdl hdl);
                    void on_close(connection_hdl hdl);
                    void on_message(connection_hdl hdl, server::message_ptr msg);

                    ViewParams params_;
                    server server_;
                    std::thread ws_run_th_;
                    std::mutex mtx_;
                    std::map<connection_hdl, std::chrono::steady_clock::time_point, std::owner_less<connection_hdl>> cnt_table_;
                };
            }
        }
    }
}
#include <functional>
#include <sstream>
#include "views.hpp"
#include "../pb/spi_message.pb.h"

namespace knit
{
    namespace actor
    {
        namespace sink
        {
            namespace websocket
            {
                View::View(const ViewParams &params) : params_(params), command_functor_(nullptr)
                {
                    ws_run_th_ = std::thread(&View::run_, this);
                }
                View::~View()
                {
                    websocketpp::lib::error_code ec;
                    std::lock_guard<std::mutex> lg(mtx_);
                    for (auto it = cnt_table_.begin(); it != cnt_table_.end(); ++it)
                    {
                        // server_.get_con_from_hdl(it->first)->close(websocket::)
                        server_.close(it->first, websocketpp::close::status::going_away, "View deconstruct", ec);
                        if (ec)
                        {
                            std::cout << "> Error initiating close: " << ec.message() << std::endl;
                        }
                    }

                    server_.stop();
                    if (ws_run_th_.joinable())
                    {
                        ws_run_th_.join();
                    }
                }

                void View::receive_loop(Tube<SPIMessagePtr> &in)
                {
                    try
                    {
                        set_status(RunStatus::Running);
                        while (get_status() != RunStatus::Terminated)
                        {
                            while (get_status() == RunStatus::Pause)
                            {
                                std::this_thread::yield();
                            }
                            receive(in);
                        }
                    }
                    catch (knit::actor::Exception e)
                    {
                        std::cout << "sink receive loop: " << e.what() << std::endl;
                        exit(0);
                    }
                }
                void View::receive(Tube<SPIMessagePtr> &in)
                {
                    auto res = in.get();
                    if (cnt_table_.empty())
                    {
                        return;
                    }

                    stm32::SPIMessage msg;
                    std::stringstream ss;
                    ss << res->name;
                    std::string name_str;
                    ss >> name_str;

                    msg.set_name(name_str);
                    msg.set_tick(res->tick);
                    auto tp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch());
                    msg.set_ts(static_cast<double>(tp.count()) / 1000.0);
                    msg.set_length(res->length);
                    msg.set_load(res->load.data(), res->load.size());
                    std::string out;
                    msg.SerializeToString(&out);
                    auto now = std::chrono::steady_clock::now();
                    std::lock_guard<std::mutex> lg(mtx_);
                    for (auto it = cnt_table_.begin(); it != cnt_table_.end();)
                    {
                        if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second) > params_.timeout)
                        {
                            std::cout << "close timeout connection" << std::endl;
                            websocketpp::lib::error_code ec;
                            auto status_code = websocketpp::close::status::normal;
                            server_.close(it->first, status_code, "heartbeat absent", ec);
                            if (ec)
                            {
                                std::cout << "> Error closing connection " << ec.message() << std::endl;
                            }
                            it = cnt_table_.erase(it);
                            continue;
                        }
                        server_.send(it->first, out, websocketpp::frame::opcode::value::binary);
                        ++it;
                    }
                }

                void View::regist_command_functor(CommandFunctor cf)
                {
                    command_functor_ = cf;
                }

                void View::run_()
                {
                    server_.set_validate_handler(std::bind(&View::validate_, this, std::placeholders::_1));
                    server_.set_open_handler(std::bind(&View::on_open, this, std::placeholders::_1));
                    server_.set_close_handler(std::bind(&View::on_close, this, std::placeholders::_1));
                    server_.set_message_handler(std::bind(&View::on_message, this, std::placeholders::_1, std::placeholders::_2));

                    server_.clear_access_channels(websocketpp::log::alevel::all);

                    server_.init_asio();
                    server_.listen(9005);
                    server_.start_accept();

                    server_.run();
                }

                bool View::validate_(connection_hdl hdl)
                {
                    server::connection_ptr con = server_.get_con_from_hdl(hdl);

                    // std::cout << "Cache-Control: " << con->get_request_header("Cache-Control") << std::endl;

                    const std::vector<std::string> &subp_requests = con->get_requested_subprotocols();
                    std::vector<std::string>::const_iterator it;

                    for (it = subp_requests.begin(); it != subp_requests.end(); ++it)
                    {
                        std::cout << "Requested: " << *it << std::endl;
                        if (*it == "spi_message")
                        {
                            std::lock_guard<std::mutex> lg(mtx_);
                            auto res = cnt_table_.find(hdl);
                            if (res == cnt_table_.end())
                            {
                                cnt_table_.insert({hdl, std::chrono::steady_clock::now()});
                            }
                            else
                            {
                                res->second = std::chrono::steady_clock::now();
                            }
                            con->select_subprotocol(*it);
                        }
                    }
                    return true;
                }
                void View::on_open(connection_hdl hdl)
                {
                    std::cout << "open a new connection" << std::endl;
                }
                void View::on_close(connection_hdl hdl)
                {
                    std::cout << "close a connection" << std::endl;
                    std::lock_guard<std::mutex> lg(mtx_);
                    auto res = cnt_table_.find(hdl);
                    if (res != cnt_table_.end())
                    {
                        cnt_table_.erase(res);
                    }
                }
                void View::on_message(connection_hdl hdl, server::message_ptr msg)
                {
                    auto payload = msg->get_payload();
                    std::cout << "on message: " << payload << std::endl;
                    std::lock_guard<std::mutex> lg(mtx_);
                    auto res = cnt_table_.find(hdl);
                    if (res != cnt_table_.end())
                    {
                        res->second = std::chrono::steady_clock::now();
                    }

                    stm32::SPIMessage spi_msg;
                    if (not spi_msg.ParseFromString(payload))
                    {
                        return;
                    }
                    if (command_functor_ != nullptr and
                        spi_msg.name() == "CommandRequest" and
                        spi_msg.has_command() and
                        spi_msg.has_timeout())
                    {
                        std::cout << "take command" << std::endl;
                        auto params = spi_msg.load();
                        std::cout << "params: " << *(uint32_t *)(params.data());
                        auto res = command_functor_(spi_msg.command(), {params.begin(), params.end()}, static_cast<uint32_t>(spi_msg.timeout() * 1000));
                        if (not res)
                        {
                            return;
                        }
                        spi_msg.set_name("CommandResponse");
                        spi_msg.set_tick(0);
                        spi_msg.set_length(res->size());
                        spi_msg.set_load(std::string(res->begin(), res->end()));
                        std::string out;
                        spi_msg.SerializeToString(&out);
                        server_.send(hdl, out, websocketpp::frame::opcode::value::binary);
                        std::cout << "return command to ws" << std::endl;
                    }
                }
            }
        }
    }
}
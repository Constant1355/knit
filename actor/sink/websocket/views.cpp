#include "views.hpp"

namespace knit
{
    namespace actor
    {
        namespace sink
        {
            namespace websocket
            {
                View::View(const ViewParams &params) : params_(params) {
                    s.set_validate_handler(bind(&validate, ref(s), ::_1));

        s.init_asio();
        s.listen(9005);
        s.start_accept();
                }
                View::~View(){

                }

                bool View::validate_(server &s, connection_hdl hdl)
                {
                    server::connection_ptr con = s.get_con_from_hdl(hdl);

                    std::cout << "Cache-Control: " << con->get_request_header("Cache-Control") << std::endl;

                    const std::vector<std::string> &subp_requests = con->get_requested_subprotocols();
                    std::vector<std::string>::const_iterator it;

                    for (it = subp_requests.begin(); it != subp_requests.end(); ++it)
                    {
                        std::cout << "Requested: " << *it << std::endl;
                    }

                    if (subp_requests.size() > 0)
                    {
                        con->select_subprotocol(subp_requests[0]);
                    }

                    return true;
                }
                void View::on_open(connection_hdl hdl)
                {
                }
                void View::on_close(connection_hdl hdl)
                {
                }
                void View::on_message(connection_hdl hdl, server::message_ptr msg)
                {
                }
            }
        }
    }
}
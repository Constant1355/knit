#include "cloud.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace ld06
            {

                CloudFrame::CloudFrame(const CloudFrameParams &params)
                    : parser_(params.filter_range){};

                void CloudFrame::action(Tube<SPIMessagePtr> &in, Tube<Laser2dCloudPtr> &out)
                {
                    Laser2dCloudPtr cloud(new LaserCloud<Laser2dPoint>);
                    float ts_bias = 0.0f;
                    while (true)
                    {
                        auto input = in.get();
                        if (input->name != source::stm32::SensorName::LD06)
                        {
                            throw Exception(ExceptionType::RUNTIME, "input msg shoule be of ld06");
                        }
                        parser_.message_parse(input, ts_bias, cloud);
                        if (cloud->points.size() > 400)
                        {
                            out.set_value(cloud);
                            break;
                        }
                    }
                }
                std::pair<size_t, size_t> CloudFrame::count() const
                {
                    return parser_.count();
                }
                uint16_t CloudFrame::speed() const
                {
                    return parser_.speed();
                }
            }
        }
    }
}
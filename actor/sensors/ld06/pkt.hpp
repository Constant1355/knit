#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include "../../source/stm32/mutex.hpp"
#include "../../common/actor.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace ld06
            {
                using source::stm32::SPIMessagePtr;

                struct Laser2dPoint
                {
                    float x;          // m
                    float y;          // m
                    float distance;   // m
                    float azimuth;    // rad
                    float confidence; // for ld06, the typical confidence of white object is about 200 in a range of 6m
                    float time_bias;  //second, the actual ts of a point is (cloud.time_base + time_bias)
                };
                template <typename T>
                struct LaserCloud
                {
                    double time_base; // second
                    std::vector<T>
                        points;
                };
                template <typename T>
                using LaserCloudPtr = std::shared_ptr<LaserCloud<T>>;

                using Laser2dCloudPtr = std::shared_ptr<LaserCloud<Laser2dPoint>>;

                class PktParser
                {
                public:
                    PktParser(const std::pair<float, float> &filter_range = {std::numeric_limits<float>::min(), std::numeric_limits<float>::max()});
                    void
                    message_parse(const SPIMessagePtr &in, float &ts_bias, Laser2dCloudPtr &cloud);
                    std::pair<size_t, size_t> count() const;
                    uint16_t speed() const; //degree/second

                private:
                    void
                    pkt_parse_(const uint8_t *data, float &ts_bias, Laser2dCloudPtr &cloud);

                    std::pair<float, float> filter_range_; // min, max
                    std::pair<size_t, size_t> count_;      //output, all
                    uint16_t laser_speed_;
                };

            }
        }
    }
}
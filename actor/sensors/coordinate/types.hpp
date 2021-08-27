#pragma once

#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include <iostream>
#include <vector>
#include <memory>
#include <Eigen/Dense>
#include "../../source/stm32/demutiplexer.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace coordinate
            {
                using source::stm32::SPIMessagePtr;

                struct IMUData
                {
                    double timestamp;
                    float temperature;
                    Eigen::Vector3f acc;  //m/s2
                    Eigen::Vector3f gyro; //rad/s
                    uint32_t count_inter;
                    uint32_t count_task;
                };
                using IMUDataPtr = std::shared_ptr<IMUData>;
                struct MagnetData
                {
                    double timestamp;
                    Eigen::Vector3f value; //uT
                };
                using MagnetDataPtr = std::shared_ptr<MagnetData>;
            }
        }
    }
}
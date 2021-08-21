#pragma once

#include <iostream>
#include "types.hpp"
#include "../../common/actor.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace coordinate
            {

                class MPU9250 : public Worker<SPIMessagePtr, IMUDataPtr>
                {
                public:
                    MPU9250() = default;
                    virtual void action(Tube<SPIMessagePtr> &in, Tube<IMUDataPtr> &out) override;
                };
            }
        }
    }
}
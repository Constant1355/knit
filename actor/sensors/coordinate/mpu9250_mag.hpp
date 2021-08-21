#pragma once

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
                class MPU9250Mag : public Worker<SPIMessagePtr, MagnetDataPtr>
                {
                public:
                    MPU9250Mag() = default;
                    virtual void action(Tube<SPIMessagePtr> &in, Tube<MagnetDataPtr> &out) override;
                };
            }
        }
    }
}
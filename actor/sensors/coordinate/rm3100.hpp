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
                class Rm3100 : public Worker<SPIMessagePtr, MagnetDataPtr>
                {
                public:
                    Rm3100() = default;
                    virtual void action(Tube<SPIMessagePtr> &in, Tube<MagnetDataPtr> &out) override;
                };
            }
        }
    }
}
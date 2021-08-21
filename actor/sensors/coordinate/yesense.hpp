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
                class Yesense : public Worker<SPIMessagePtr, IMUDataPtr>
                {
                public:
                    Yesense() = default;
                    virtual void action(Tube<SPIMessagePtr> &in, Tube<IMUDataPtr> &out) override;
                };
            }
        }
    }
}
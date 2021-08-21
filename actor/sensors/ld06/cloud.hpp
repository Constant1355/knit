#pragma once

#include "../../common/actor.hpp"
#include "pkt.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace ld06
            {
                struct CloudFrameParams
                {
                    std::pair<float, float> filter_range;
                };

                class CloudFrame : public Worker<SPIMessagePtr, Laser2dCloudPtr>
                {
                public:
                    CloudFrame(const CloudFrameParams &params);
                    virtual void action(Tube<SPIMessagePtr> &in, Tube<Laser2dCloudPtr> &out) override;
                    std::pair<size_t, size_t> count() const;
                    uint16_t speed() const; //degree/second

                private:
                    CloudFrameParams params_;

                    PktParser parser_;
                };

                class CloudStream
                {
                };
            }
        }
    }
}
#pragma once

#include <ctime>
#include <list>
#include <optional>
#include "../../source/stm32/spi.hpp"
#include "../../common/actor.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace clock
            {
                using source::stm32::SPIMessagePtr;

                struct DS3231Data
                {
                    double stm32_on_second; //stm32 ticks when ds3231 starts from a second
                    double stm32_on_i2c;    //stm32 ticks when starts i2c
                    uint32_t ds_seconds;    //seconds count from ds3231 32k
                    struct tm date;
                };

                class DS3231 : public Sink<SPIMessagePtr>
                {
                public:
                    DS3231(const size_t buffer_length = 600);
                    virtual void receive(Tube<SPIMessagePtr> &in) override;

                    std::optional<DS3231Data> latest();

                private:
                    std::mutex buffer_mtx_;
                    std::list<DS3231Data> timestamp_buffer_;
                    const size_t buffer_length_;
                };
            }
        }
    }
}
#pragma once

#include "sqlite3.h"
#include "../../common/actor.hpp"
#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

namespace knit
{
    namespace actor
    {
        namespace sink
        {
            namespace dump
            {
                using source::stm32::SPIMessagePtr;
                struct DumpParams
                {
                    std::vector<source::stm32::SensorName> dump_sensors;
                    std::string db;
                };
                class Dump : public Sink<SPIMessagePtr>
                {
                public:
                    Dump(const DumpParams &params);
                    ~Dump();

                    virtual void receive(Tube<SPIMessagePtr> &in) override;

                private:
                    DumpParams params_;
                    sqlite3 *db_;
                    sqlite3_stmt *stmt_;
                    size_t count_;
                };
            }
        }
    }
}
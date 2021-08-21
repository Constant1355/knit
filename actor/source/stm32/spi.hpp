#pragma once

#include <iostream>
#include <vector>
#include <future>
#include <mutex>
#include <map>
#include <set>
#include <utility>
#include <linux/spi/spidev.h>
#include "../../common/actor.hpp"

namespace knit
{
    namespace actor
    {
        namespace source
        {
            namespace stm32
            {

                enum class SensorName
                {
                    LD06,
                    YESENSE,
                    MPU9250,
                    MPU9250_MAG,
                    DS3231,
                    RM3100,
                    UNKNOWN
                };

                std::ostream &operator<<(std::ostream &os, SensorName const &c);

                extern const std::map<SensorName, size_t> SensorNamesMarkedBit;

                struct SPIHeader
                {
                    uint8_t magic_head; // 0xee
                    uint8_t valid;
                    uint8_t crc;
                    uint8_t sensor_name;
                    uint32_t tick; // ms
                    uint32_t offset;
                    uint32_t length;
                };

                struct SPIMessage
                {
                    SensorName name;
                    uint32_t length;
                    uint32_t tick; // ms
                    std::vector<uint8_t> load;
                    void reset();
                    friend std::ostream &operator<<(std::ostream &os, const SPIMessage &c);
                };
                using SPIMessagePtr = std::shared_ptr<SPIMessage>;

                struct SPIParmas
                {
                    std::string spi_dev_name;
                    int spi_bytes;
                    int spi_clock_hz;
                };

                class SPI : public Source<SPIMessagePtr, 10>
                {
                public:
                    SPI(const SPIParmas &params);
                    ~SPI();
                    virtual void send(std::vector<MarkedTube> &out) override;
                    void emplace_tube(const std::vector<SensorName> &sensors, std::vector<MarkedTube> &tubes) const;

                private:
                    void init_();
                    void read_write_(const int &len);

                    SPIParmas params_;
                    int spi_file_;
                    const uint32_t single_load_length_;

                    std::mutex rw_mtx_;

                    std::vector<uint8_t> tx_, rx_;
                };
            }
        }
    }
}
#pragma once

#include <iostream>
#include <vector>
#include <future>
#include <list>
#include <map>
#include <set>
#include <mutex>
#include <thread>
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

                constexpr int MarkerSize = 20;

                enum class SensorName
                {
                    LD06,
                    YESENSE,
                    MPU9250,
                    MPU9250_MAG,
                    DS3231,
                    RM3100,
                    E108,
                    Battery_VC,
                    Heds,
                    CommandResponse,
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

                struct SPICommand
                {
                    uint32_t action;
                    uint32_t sequence;
                    std::vector<uint8_t> params;
                    std::promise<std::vector<uint8_t>> promise_response;
                    bool status_wait;
                };
                using SPICommandPtr = std::shared_ptr<SPICommand>;

                struct SPIParmas
                {
                    std::string spi_dev_name;
                    int spi_bytes;
                    int spi_clock_hz;
                };

                class SPI : public Source<SPIMessagePtr, MarkerSize>
                {
                public:
                    SPI(const SPIParmas &params);
                    ~SPI();
                    virtual void send(std::vector<MarkedTube> &out) override;

                    std::optional<std::vector<uint8_t>> command(
                        const uint32_t &action,
                        const std::vector<uint8_t> &params,
                        const uint32_t &timeout_milliseconds = 1000);
                    void emplace_tube(const std::vector<SensorName> &sensors, std::vector<MarkedTube> &tubes) const;

                private:
                    void init_();
                    void read_write_loop_();
                    void fill_tx_();
                    bool sychronous_head_();
                    void transfer_bytes_(uint len);
                    bool parse_rx_(SPIMessagePtr &msg);

                    SPIParmas params_;
                    int spi_file_;
                    uint32_t cmd_seq_;
                    const uint32_t single_load_length_;

                    Tube<SPIMessagePtr> rx_buffer_;
                    std::mutex cmds_mtx_;
                    std::list<SPICommandPtr> cmds_;
                    std::vector<uint8_t> tx_, rx_;
                    std::thread rx_loop_th_;
                };
            }
        }
    }
}
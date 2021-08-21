#include "ds3231.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace clock
            {
                DS3231::DS3231(const size_t buffer_length) : buffer_length_(buffer_length) {}

                void DS3231::receive(Tube<SPIMessagePtr> &in)
                {
                    auto input = in.get();
                    if (input->name != source::stm32::SensorName::DS3231)
                    {
                        throw Exception(ExceptionType::RUNTIME, "input msg shoule be of ds3231");
                    }

                    // machine tick on second + DS3231 seconds since count + machine tick on i2c + DS3231_Regs_Map
                    // #define DS3231_PKT_LEN (4 + 4 + 4 + 0x13)
                    DS3231Data data;
                    auto ptr = input->load.data();
                    data.stm32_on_second = static_cast<float>(*(reinterpret_cast<uint32_t *>(ptr))) / 1000.0f;
                    data.ds_seconds = *(reinterpret_cast<uint32_t *>(ptr + 4));
                    data.stm32_on_i2c = static_cast<float>(*(reinterpret_cast<uint32_t *>(ptr + 8))) / 1000.0f;

                    auto date_ptr = ptr + 12;
                    data.date.tm_sec = 10 * ((date_ptr[0] & 0x70) >> 4) + (date_ptr[0] & 0x0f);
                    data.date.tm_min = 10 * ((date_ptr[1] & 0x70) >> 4) + (date_ptr[1] & 0x0f);
                    const auto &H = date_ptr[2];

                    if (H & 0x40)
                    {
                        // 12h
                        data.date.tm_hour = 10 * ((H & 0x10) >> 4) + (H & 0x0f);
                        if (H & 0x20)
                        {
                            // pm
                            data.date.tm_hour += 12;
                        }
                    }
                    else
                    {
                        // 24h
                        data.date.tm_hour = 10 * ((H & 0x30) >> 4) + (H & 0x0f);
                    }
                    data.date.tm_wday = (date_ptr[3] & 0x0f) - 1;
                    data.date.tm_mday = 10 * ((date_ptr[4] & 0x30) >> 4) + (date_ptr[4] & 0x0f) + 1;
                    data.date.tm_mon = 10 * ((date_ptr[4] & 0x10) >> 4) + (date_ptr[4] & 0x0f) - 1;
                    data.date.tm_year = 10 * ((date_ptr[4] & 0xf0) >> 4) + (date_ptr[4] & 0x0f) + 100;

                    // std::cout << "date_ptr[4]: " << std::hex << (int)date_ptr[4] << std::dec << std::endl;
                    std::lock_guard<std::mutex> lg(buffer_mtx_);
                    timestamp_buffer_.push_back(data);
                    if (timestamp_buffer_.size() > buffer_length_)
                    {
                        timestamp_buffer_.pop_front();
                    }
                }

                std::optional<DS3231Data> DS3231::latest()
                {
                    if (timestamp_buffer_.empty())
                    {
                        return std::nullopt;
                    }
                    std::lock_guard<std::mutex> lg(buffer_mtx_);
                    return timestamp_buffer_.back();
                }
            }
        }
    }
}
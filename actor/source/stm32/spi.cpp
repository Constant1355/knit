#include "spi.hpp"

#include <vector>
#include <map>
#include <chrono>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "../../common/exception.hpp"

namespace knit
{
    namespace actor
    {
        namespace source
        {
            namespace stm32
            {
                static const uint8_t CrcTable[512] = {
                    0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3,
                    0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33,
                    0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8,
                    0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77,
                    0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55,
                    0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4,
                    0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f,
                    0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff,
                    0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2,
                    0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12,
                    0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99,
                    0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14,
                    0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36,
                    0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9,
                    0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72,
                    0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2,
                    0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1,
                    0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71,
                    0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa,
                    0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35,
                    0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17,
                    0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8, 0x00, 0x4d, 0x9a,
                    0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11,
                    0x5c, 0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1,
                    0x8c, 0x22, 0x6f, 0xb8, 0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc,
                    0xb1, 0xed, 0xa0, 0x77, 0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c,
                    0x61, 0xcf, 0x82, 0x55, 0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7,
                    0xea, 0x3e, 0x73, 0xa4, 0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56,
                    0x1b, 0xb5, 0xf8, 0x2f, 0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74,
                    0x39, 0x65, 0x28, 0xff, 0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb,
                    0xf6, 0x58, 0x15, 0xc2, 0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30,
                    0x7d, 0x88, 0xc5, 0x12, 0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0,
                    0xad, 0x03, 0x4e, 0x99, 0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f,
                    0xd2, 0x8e, 0xc3, 0x14, 0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f,
                    0x02, 0xac, 0xe1, 0x36, 0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4,
                    0x89, 0x63, 0x2e, 0xf9, 0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b,
                    0x46, 0xe8, 0xa5, 0x72, 0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29,
                    0x64, 0x38, 0x75, 0xa2, 0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8,
                    0x95, 0x3b, 0x76, 0xa1, 0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53,
                    0x1e, 0xeb, 0xa6, 0x71, 0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83,
                    0xce, 0x60, 0x2d, 0xfa, 0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe,
                    0xf3, 0xaf, 0xe2, 0x35, 0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e,
                    0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5,
                    0xa8};

                const std::map<SensorName, size_t> SensorNamesMarkedBit{
                    std::make_pair(SensorName::LD06, 0x01),
                    std::make_pair(SensorName::YESENSE, 0x02),
                    std::make_pair(SensorName::MPU9250, 0x03),
                    std::make_pair(SensorName::MPU9250_MAG, 0x06),
                    std::make_pair(SensorName::DS3231, 0x04),
                    std::make_pair(SensorName::RM3100, 0x05),
                    std::make_pair(SensorName::E108, 0x07),
                    std::make_pair(SensorName::Battery_VC, 0x08),
                    std::make_pair(SensorName::Heds, 0x09),
                    std::make_pair(SensorName::CommandResponse, 0x10),
                };

                const uint8_t SPIMessageHead = 0xee;
                const int SPIMessageHeadLength = sizeof(SPIHeader);

                spi_ioc_transfer xfer_;

                void SPIMessage::reset()
                {
                    name = SensorName::UNKNOWN;
                    length = 0;
                    load.clear();
                }

                std::ostream &operator<<(std::ostream &os, SensorName const &c)
                {
                    switch (c)
                    {
                    case SensorName::LD06:
                        os << "LD06";
                        break;
                    case SensorName::YESENSE:
                        os << "YESENSE";
                        break;
                    case SensorName::MPU9250:
                        os << "MPU9250";
                        break;
                    case SensorName::MPU9250_MAG:
                        os << "MPU9250_MAG";
                        break;
                    case SensorName::DS3231:
                        os << "DS3231";
                        break;
                    case SensorName::RM3100:
                        os << "RM3100";
                        break;
                    case SensorName::E108:
                        os << "E108";
                        break;
                    case SensorName::Battery_VC:
                        os << "Battery_VC";
                        break;
                    case SensorName::Heds:
                        os << "Heds";
                        break;
                    case SensorName::CommandResponse:
                        os << "CommandResponse";
                        break;

                    default:
                        os << "UNKNOWN";
                        break;
                    }
                    return os;
                }

                std::ostream &operator<<(std::ostream &os, const SPIMessage &c)
                {
                    os << c.name << " " << c.tick << " " << c.length << " " << c.load.size();
                    return os;
                }

                SPI::SPI(const SPIParmas &params)
                    : params_(params),
                      spi_file_(-1), cmd_seq_(0),
                      single_load_length_(params_.spi_bytes - SPIMessageHeadLength)
                {
                    init_();
                }

                SPI::~SPI()
                {
                    if (spi_file_ != -1)
                    {
                        close(spi_file_);
                    }
                    spi_file_ = -1;
                    if (rx_loop_th_.joinable())
                    {
                        rx_loop_th_.join();
                    }
                }

                void SPI::send(std::vector<MarkedTube> &out)
                {
                    auto msg = rx_buffer_.get();
                    auto res_marker = SensorNamesMarkedBit.find(msg->name);
                    if (res_marker == SensorNamesMarkedBit.end())
                    {
                        throw Exception(ExceptionType::RUNTIME,
                                        "spi message sensor name marker error ");
                    }
                    for (auto &markerd_tube : out)
                    {
                        if (markerd_tube.first.test(res_marker->second))
                        {
                            markerd_tube.second->set_value(msg);
                        }
                    }
                }

                std::optional<std::vector<uint8_t>> SPI::command(const SPICommandPtr &cmd, const uint32_t &timeout_milliseconds)
                {
                    std::pair<std::map<uint32_t, std::promise<std::vector<uint8_t>>>::iterator, bool>
                        it;
                    {
                        std::lock_guard<std::mutex> lg(tx_buffer_mtx_);
                        it = cmd_response_.emplace(std::piecewise_construct,
                                                   std::forward_as_tuple(cmd_seq_),
                                                   std::forward_as_tuple(std::promise<std::vector<uint8_t>>()));
                        if (not it.second)
                        {
                            return std::nullopt;
                        }
                        tx_buffer_.push({cmd_seq_, cmd});
                        ++cmd_seq_;
                    }

                    auto res_f = it.first->second.get_future();
                    auto res = res_f.wait_for(std::chrono::milliseconds(timeout_milliseconds));

                    std::vector<uint8_t> response_data;
                    std::lock_guard<std::mutex> lg(tx_buffer_mtx_);
                    if (res != std::future_status::ready)
                    {
                        cmd_response_.erase(it.first);
                        return std::nullopt;
                    }
                    response_data = res_f.get();
                    cmd_response_.erase(it.first);
                    return response_data;
                }

                void SPI::emplace_tube(const std::vector<SensorName> &sensors, std::vector<MarkedTube> &tubes) const
                {
                    std::bitset<MarkerSize> marker;
                    for (const auto &s : sensors)
                    {
                        auto res = SensorNamesMarkedBit.find(s);
                        if (res != SensorNamesMarkedBit.end())
                        {
                            marker.set(res->second);
                        }
                    }
                    tubes.emplace_back(
                        std::piecewise_construct,
                        std::forward_as_tuple(marker),
                        std::forward_as_tuple(new Tube<SPIMessagePtr>));
                }

                void SPI::init_()
                {
                    if (spi_file_ != -1)
                    {
                        throw Exception(ExceptionType::INITIAL, "spi file != -1");
                    }
                    if ((spi_file_ = open(params_.spi_dev_name.c_str(), O_RDWR)) < 0)
                    {
                        spi_file_ = -1;
                        throw Exception(ExceptionType::INITIAL, "spi open error with dev name: " + params_.spi_dev_name);
                    }

                    unsigned long mode = SPI_MODE_0;
                    if (ioctl(spi_file_, SPI_IOC_WR_MODE, &mode) < 0)
                    {
                        throw Exception(ExceptionType::INITIAL, "spi faild to set mode: " + std::to_string(mode));
                    }

                    xfer_.speed_hz = params_.spi_clock_hz;
                    xfer_.len = params_.spi_bytes;
                    xfer_.cs_change = 0;
                    xfer_.delay_usecs = 0;
                    xfer_.bits_per_word = 8;

                    if (ioctl(spi_file_, SPI_IOC_WR_MAX_SPEED_HZ, &xfer_.speed_hz) < 0)
                    {
                        throw Exception(ExceptionType::INITIAL, "spi faild to set max speed: " + std::to_string(xfer_.speed_hz));
                    }

                    tx_.resize(params_.spi_bytes, 0);
                    rx_.resize(params_.spi_bytes, 0);
                    xfer_.tx_buf = (unsigned long)tx_.data();
                    xfer_.rx_buf = (unsigned long)rx_.data();

                    rx_loop_th_ = std::thread(&SPI::read_write_loop_, this);
                }

                void SPI::read_write_loop_()
                {
                    bool sychronous = false;
                    SPIMessagePtr msg(new SPIMessage());
                    while (spi_file_ != -1)
                    {
                        tx_.clear();
                        tx_.resize(params_.spi_bytes, 0);
                        if (sychronous)
                        {
                            fill_tx_();
                        }
                        transfer_bytes_(params_.spi_bytes);
                        if (rx_[0] != SPIMessageHead)
                        {
                            sychronous = false;
                            if (not sychronous_head_())
                            {
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                std::cout << "sychronize magic_head" << std::endl;
                                continue;
                            }
                        }
                        sychronous = true;
                        if (not parse_rx_(msg))
                        {
                            std::this_thread::yield();
                            continue;
                        }
                    }
                }

                void SPI::fill_tx_()
                {
                    std::lock_guard<std::mutex> lg(tx_buffer_mtx_);
                    if (not tx_buffer_.empty())
                    {
                        auto front = tx_buffer_.front();
                        tx_buffer_.pop();
                        const auto &params = front.second->params;
                        auto params_size = static_cast<uint32_t>(params.size());
                        if (params_size > params_.spi_bytes - 14)
                        {
                            throw Exception(ExceptionType::RUNTIME,
                                            "spi tx buffer overflow with load size: " +
                                                std::to_string(params_size));
                        }
                        tx_[0] = SPIMessageHead;
                        *reinterpret_cast<uint32_t *>(tx_.data() + 1) = front.second->action;
                        *reinterpret_cast<uint32_t *>(tx_.data() + 5) = front.first;
                        *reinterpret_cast<uint32_t *>(tx_.data() + 9) = params_size;

                        std::copy(params.begin(), params.end(), tx_.begin() + 13);

                        uint8_t crc_calculated = 0;
                        for (int idx = 0; idx < params_size + 13; ++idx)
                        {
                            crc_calculated = CrcTable[(crc_calculated ^ tx_[idx]) & 0xff];
                        }
                        tx_[params_size + 13] = crc_calculated;
                    }
                }

                bool SPI::sychronous_head_()
                {
                    int idx = 0;
                    for (; idx < params_.spi_bytes; ++idx)
                    {
                        if (rx_[idx] == SPIMessageHead)
                        {
                            transfer_bytes_(1);
                            break;
                        }
                    }
                    return idx != params_.spi_bytes;
                }

                void SPI::transfer_bytes_(uint len)
                {
                    xfer_.len = len;
                    auto status = ioctl(spi_file_, SPI_IOC_MESSAGE(1), &xfer_);
                    if (status < 0)
                    {
                        throw Exception(ExceptionType::RUNTIME, "spi IO error");
                    }
                }

                bool SPI::parse_rx_(SPIMessagePtr &msg)
                {
                    auto header_ptr = reinterpret_cast<SPIHeader *>(rx_.data());
                    if (header_ptr->valid != 1)
                    {
                        return false;
                    }

                    uint8_t crc_calculated = 0;
                    for (int idx = 3; idx < params_.spi_bytes; ++idx)
                    {
                        crc_calculated = CrcTable[(crc_calculated ^ rx_[idx]) & 0xff];
                    }
                    if (header_ptr->crc != crc_calculated)
                    {
                        std::cout << "crc error" << std::endl;
                        return false;
                    }
                    if (header_ptr->offset == 0)
                    {
                        msg = std::make_shared<SPIMessage>();
                    }

                    switch (header_ptr->sensor_name)
                    {
                    case 0x01:
                        msg->name = SensorName::LD06;
                        break;
                    case 0x02:
                        msg->name = SensorName::YESENSE;
                        break;
                    case 0x03:
                        msg->name = SensorName::MPU9250;
                        break;
                    case 0x04:
                        msg->name = SensorName::DS3231;
                        break;
                    case 0x05:
                        msg->name = SensorName::RM3100;
                        break;
                    case 0x06:
                        msg->name = SensorName::MPU9250_MAG;
                        break;
                    case 0x07:
                        msg->name = SensorName::E108;
                        break;
                    case 0x08:
                        msg->name = SensorName::Battery_VC;
                        break;
                    case 0x09:
                        msg->name = SensorName::Heds;
                        break;
                    case 0xf0:
                        msg->name = SensorName::CommandResponse;
                        break;
                    default:
                        msg->name = SensorName::UNKNOWN;
                        std::cout << "skip UNKNOWN SensorName" << std::endl;
                        throw Exception(ExceptionType::RUNTIME,
                                        "spi message sensor name error: " + std::to_string(rx_[2]));
                    }

                    auto load_begin = rx_.begin() + SPIMessageHeadLength;

                    if (header_ptr->length - header_ptr->offset > single_load_length_)
                    {
                        std::copy(load_begin, rx_.end(), std::back_inserter(msg->load));
                    }
                    else
                    {
                        auto load_end = load_begin + header_ptr->length - header_ptr->offset;
                        std::copy(load_begin, load_end, std::back_inserter(msg->load));
                    }
                    auto load_length = msg->load.size();
                    if (load_length > header_ptr->length)
                    {
                        std::cout << "load_length: " << load_length << "; " << header_ptr->length << std::endl;
                        throw Exception(ExceptionType::RUNTIME, "spi message overflow error ");
                    }
                    else if (load_length < header_ptr->length)
                    {
                        return true;
                    }

                    msg->length = header_ptr->length;
                    msg->tick = header_ptr->tick;
                    // std::cout << "final: " << msg->length << " should " << msg->load.size() << " tick: " << msg->tick << std::endl;

                    rx_buffer_.set_value(msg);
                    if (msg->name == SensorName::CommandResponse and msg->load.size() >= 4)
                    {
                        const auto &seq = *reinterpret_cast<uint32_t *>(msg->load.data());

                        std::lock_guard<std::mutex> lg(tx_buffer_mtx_);
                        auto res = cmd_response_.find(seq);
                        if (res != cmd_response_.end())
                        {
                            res->second.set_value({msg->load.begin() + 4, msg->load.end()});
                        }
                    }
                    return true;
                }
            }
        }
    }
}

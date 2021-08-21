#include <math.h>
#include "pkt.hpp"
#include "../../common/exception.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace ld06
            {
                enum
                {
                    PKG_HEADER = 0x54,
                    PKG_VER_LEN = 0x2C,
                    POINT_PER_PACK = 12,
                };

                typedef struct __attribute__((packed))
                {
                    uint16_t distance;
                    uint8_t confidence;
                } LidarPointStructDef;

                typedef struct __attribute__((packed))
                {
                    uint8_t header;
                    uint8_t ver_len;
                    uint16_t speed; // degree/seond
                    uint16_t start_angle;
                    LidarPointStructDef point[POINT_PER_PACK];
                    uint16_t end_angle;
                    uint16_t timestamp;
                    uint8_t crc8;
                } LiDARFrameTypeDef;

                static const uint8_t CrcTable[256] =
                    {
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
                        0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8};

                const float TimeInterval = 1.0f / 4500.0f; // second

                PktParser::PktParser(const std::pair<float, float> &filter_range)
                    : filter_range_(filter_range), count_{0, 0} {}

                void
                PktParser::message_parse(const SPIMessagePtr &in, float &ts_bias, Laser2dCloudPtr &cloud)
                {
                    if (cloud->points.empty())
                    {
                        ts_bias = 0.0f;
                        cloud->time_base = static_cast<double>(in->tick) / 1000.0;
                    }
                    for (int idx = sizeof(LiDARFrameTypeDef);
                         idx < in->length;
                         idx += sizeof(LiDARFrameTypeDef))
                    {
                        pkt_parse_(in->load.data() + (idx - sizeof(LiDARFrameTypeDef)), ts_bias, cloud);
                    }
                }
                std::pair<size_t, size_t> PktParser::count() const
                {
                    return count_;
                }
                uint16_t PktParser::speed() const
                {
                    return laser_speed_;
                }

                void
                PktParser::pkt_parse_(const uint8_t *data, float &ts_bias, Laser2dCloudPtr &cloud)
                {
                    auto frame = (LiDARFrameTypeDef *)data;
                    if (frame->header != PKG_HEADER or frame->ver_len != PKG_VER_LEN)
                    {
                        return;
                    }
                    // crc validate
                    uint8_t crc = 0;
                    for (uint32_t i = 0; i < sizeof(LiDARFrameTypeDef) - 1; i++)
                    {
                        crc = CrcTable[(crc ^ data[i]) & 0xff];
                    }
                    if (crc != frame->crc8)
                    {
                        // std::cout << "pkt: " << std::hex;
                        // for (int idx = 0; idx < sizeof(LiDARFrameTypeDef); ++idx)
                        // {
                        //     std::cout << (int)(data[idx]) << ", ";
                        // }
                        // std::cout << std::endl;
                        throw Exception(ExceptionType::RUNTIME, "ld06 pkt crc error");
                    }
                    laser_speed_ = frame->speed;

                    auto angle_start = static_cast<float>(frame->start_angle) / 18000.f * M_PIf32;
                    auto angle_end = static_cast<float>(frame->end_angle) / 18000.f * M_PIf32;
                    if (frame->end_angle < frame->start_angle)
                    {
                        angle_end += 2 * M_PIf32;
                    }
                    auto angle_step = ((angle_end - angle_start) / static_cast<float>(POINT_PER_PACK - 1));
                    for (int idx = 0; idx < POINT_PER_PACK; ++idx)
                    {

                        auto az = angle_start + angle_step * static_cast<float>(idx);
                        auto dist = static_cast<float>(frame->point[idx].distance) * .001f;

                        count_.second += 1;
                        if (dist > filter_range_.second or dist < filter_range_.first)
                        {
                            continue;
                        }
                        count_.first += 1;

                        cloud->points.push_back({dist * cosf(az),                                    // x
                                                 dist * sinf(az),                                    // y
                                                 dist,                                               // distance
                                                 az,                                                 // azimuth
                                                 static_cast<float>(frame->point[idx].confidence),   // confidence
                                                 static_cast<float>(idx) * TimeInterval + ts_bias}); // time_bias
                    }
                    ts_bias += static_cast<float>(POINT_PER_PACK) * TimeInterval;
                }
            }
        }
    }
}
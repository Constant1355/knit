#include "mpu9250.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace coordinate
            {
                const float gravity = 9.8f;
                const std::vector<float> MPU9250AccMeasurements{16384.0f / gravity, 8192.0f / gravity, 4096.0f / gravity, 2048.0f / gravity};
                const std::vector<float> MPU9250GyroMeasurements{131.0f, 65.5f, 32.8f, 16.4f};
                const size_t PktLength = 28;

                void MPU9250::action(Tube<SPIMessagePtr> &in, Tube<IMUDataPtr> &out)
                {
                    auto input = in.get();
                    if (input->name != source::stm32::SensorName::MPU9250)
                    {
                        throw Exception(ExceptionType::RUNTIME, "input msg shoule be of mpu9250");
                    }

                    for (size_t idx = PktLength; idx <= input->length; idx += PktLength)
                    {

                        IMUDataPtr data(new IMUData);
                        // machine tick + gyro_acc_config + measure acc_xyz + temperature + gyro_xyz
                        // #define MPU9250_PKT_LEN (4 + 2 + 6 + 2 + 6)
                        auto pptr = input->load.data() + (idx - PktLength);
                        data->timestamp = static_cast<float>(*(reinterpret_cast<uint32_t *>(pptr))) / 1000.0f;
                        data->count_inter = *((reinterpret_cast<uint32_t *>(pptr + 4)));
                        data->count_task = *((reinterpret_cast<uint32_t *>(pptr + 8)));

                        auto ptr = pptr + 8;
                        // std::cout << "idx: " << idx << "; timestamp:　" << data->timestamp << std::endl;
                        pulse.first = data->timestamp;
                        ++pulse.second;
                        const auto &gyro_cfg = ptr[4];
                        const auto &acc_cfg = ptr[5];
                        if (3 < gyro_cfg or 3 < acc_cfg)
                        {
                            throw Exception(ExceptionType::RUNTIME, "mpu9250 msg config error");
                        }
                        auto gyro_m = MPU9250GyroMeasurements[gyro_cfg];
                        auto acc_m = MPU9250AccMeasurements[acc_cfg];

                        auto ptr16 = (int16_t *)(ptr + 6);
                        for (int idx = 6; idx < PktLength - 8; idx += 2)
                        {
                            std::swap(ptr[idx], ptr[idx + 1]);
                        }
                        data->acc << static_cast<float>(ptr16[0]) / acc_m,
                            static_cast<float>(ptr16[1]) / acc_m,
                            static_cast<float>(ptr16[2]) / acc_m;
                        data->temperature = (static_cast<float>(ptr16[3] + 521) / 340.0f);
                        data->gyro << static_cast<float>(ptr16[4]) / acc_m,
                            static_cast<float>(ptr16[5]) / acc_m,
                            static_cast<float>(ptr16[6]) / acc_m;

                        out.set_value(data);
                    }
                }

            }
        }
    }
}
#include "mpu9250_mag.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace coordinate
            {
                const size_t PktLength = 14;

                void MPU9250Mag::action(Tube<SPIMessagePtr> &in, Tube<MagnetDataPtr> &out)
                {
                    auto input = in.get();
                    if (input->name != source::stm32::SensorName::MPU9250_MAG)
                    {
                        throw Exception(ExceptionType::RUNTIME, "input msg shoule be of mpu9250_mag");
                    }

                    for (size_t idx = PktLength; idx <= input->length; idx += PktLength)
                    {

                        MagnetDataPtr data(new MagnetData);
                        //machine tick + hxyz + st2 + asa
                        // #define MPU9250_Meg_PKT_LEN (4 + 6 + 1 + 3)
                        auto ptr = input->load.data() + (idx - PktLength);
                        data->timestamp = static_cast<float>(*(reinterpret_cast<uint32_t *>(ptr))) / 1000.0f;

                        auto hx = static_cast<float>(*(int16_t *)(ptr + 4));
                        auto hy = static_cast<float>(*(int16_t *)(ptr + 6));
                        auto hz = static_cast<float>(*(int16_t *)(ptr + 8));

                        int asa[3] = {
                            (int)ptr[11],
                            (int)ptr[12],
                            (int)ptr[13]};

                        float coe[3] = {
                            (asa[0] - 128) * 0.5f / 128.0f,
                            (asa[1] - 128) * 0.5f / 128.0f,
                            (asa[2] - 128) * 0.5f / 128.0f,
                        };

                        hx *= (1.0f + coe[0]);
                        hy *= (1.0f + coe[1]);
                        hz *= (1.0f + coe[2]);

                        data->value << hx, hy, hz;

                        out.set_value(data);
                    }
                }
            }
        }
    }
}
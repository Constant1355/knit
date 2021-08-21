#include "yesense.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace coordinate
            {
                const size_t PktLength = 38;
                const float AccCoe = 0.000001f;
                const float GyroCoe = 0.000001f;
                const float TemperatureCoe = 0.01f;

                void Yesense::action(Tube<SPIMessagePtr> &in, Tube<IMUDataPtr> &out)
                {
                    auto input = in.get();
                    if (input->name != source::stm32::SensorName::YESENSE)
                    {
                        throw Exception(ExceptionType::RUNTIME, "input msg shoule be of yesense");
                    }

                    for (size_t idx = PktLength; idx <= input->length; idx += PktLength)
                    {

                        IMUDataPtr data(new IMUData);
                        // machine tick + temperature + acc + gyro
                        // #define YESENSE_PKT_LEN (4 + 2 + 12 + 12)
                        auto ptr = input->load.data() + (idx - PktLength);
                        data->timestamp = static_cast<float>(*(reinterpret_cast<uint32_t *>(ptr))) / 1000.0f;
                        pulse.first = data->timestamp;
                        ++pulse.second;
                        data->count_inter = *(reinterpret_cast<uint32_t *>(ptr + 4));
                        data->count_task = *(reinterpret_cast<uint32_t *>(ptr + 8));
                        data->temperature = static_cast<float>(*reinterpret_cast<int16_t *>(ptr + 12)) * TemperatureCoe;

                        auto ptr32 = (int32_t *)(ptr + 14);

                        data->acc << static_cast<float>(ptr32[0]) * AccCoe,
                            static_cast<float>(ptr32[1]) * AccCoe,
                            static_cast<float>(ptr32[2]) * AccCoe;
                        data->gyro << static_cast<float>(ptr32[3]) * GyroCoe,
                            static_cast<float>(ptr32[4]) * GyroCoe,
                            static_cast<float>(ptr32[5]) * GyroCoe;

                        out.set_value(data);
                    }
                }
            }
        }
    }
}
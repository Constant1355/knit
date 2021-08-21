#include "rm3100.hpp"

namespace knit
{
    namespace actor
    {
        namespace sensor
        {
            namespace coordinate
            {
                const size_t PktLength = 14;
                const float ValueCoe = 1.0f / 75.0f / 200.0f;

                void Rm3100::action(Tube<SPIMessagePtr> &in, Tube<MagnetDataPtr> &out)
                {
                    auto input = in.get();
                    if (input->name != source::stm32::SensorName::RM3100)
                    {
                        throw Exception(ExceptionType::RUNTIME, "input msg shoule be of rm3100");
                    }

                    for (size_t idx = PktLength; idx <= input->length; idx += PktLength)
                    {

                        MagnetDataPtr data(new MagnetData);
                        // machine tick + measure xyz
                        // #define RM3100_PKT_LEN (4 + 1 + 3 * 3)
                        auto ptr = input->load.data() + (idx - PktLength);
                        data->timestamp = static_cast<float>(*(reinterpret_cast<uint32_t *>(ptr))) / 1000.0f;

                        int32_t mxyz[3];
                        mxyz[0] = *((int16_t *)(ptr + 5));
                        mxyz[1] = *((int16_t *)(ptr + 8));
                        mxyz[2] = *((int16_t *)(ptr + 11));

                        mxyz[0] <<= 8;
                        mxyz[1] <<= 8;
                        mxyz[2] <<= 8;

                        mxyz[0] |= *(ptr + 7);
                        mxyz[1] |= *(ptr + 10);
                        mxyz[2] |= *(ptr + 13);

                        data->value << static_cast<float>(mxyz[0]) * ValueCoe,
                            static_cast<float>(mxyz[1]) * ValueCoe,
                            static_cast<float>(mxyz[2]) * ValueCoe;

                        out.set_value(data);
                    }
                }
            }
        }
    }
}
#include "../../sensors/coordinate/mpu9250.hpp"
#include "../../sensors/coordinate/yesense.hpp"
#include "../../source/stm32/mutex.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::sensor::coordinate;
using namespace knit::actor::source::stm32;

int main()
{
    SPIParmas spi_params;
    spi_params.spi_bytes = 400;
    spi_params.spi_clock_hz = 3000000;
    spi_params.spi_dev_name = "/dev/spidev0.0";
    std::thread spi_loop_th, cloud_loop_th;

    try
    {

        SPI spi(spi_params);
        std::vector<SPI::MarkedTube> tubes;
        // spi.emplace_tube({SensorName::MPU9250}, tubes);
        spi.emplace_tube({SensorName::YESENSE}, tubes);
        spi_loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));

        // MPU9250 mpu9250;
        Yesense yesense;

        knit::actor::Tube<IMUDataPtr> out;
        {
            // cloud_loop_th = std::thread(&MPU9250::action_loop, &mpu9250, std::ref(*tubes[0].second), std::ref(out));
            cloud_loop_th = std::thread(&Yesense::action_loop, &yesense, std::ref(*tubes[0].second), std::ref(out));
        }
        while (true)
        {
            auto res = out.get();
            std::cout << "timestamp: "
                      << res->timestamp
                      << ", acc: "
                      << res->acc.transpose()
                      << ", gyro: "
                      << res->gyro.transpose()
                      << ", temperature: "
                      << res->temperature
                      << std::endl;
        }
    }
    catch (knit::actor::Exception e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
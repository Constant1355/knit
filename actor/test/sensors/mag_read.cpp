#include "../../sensors/coordinate/rm3100.hpp"
#include "../../sensors/coordinate/mpu9250_mag.hpp"
#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::sensor::coordinate;
using namespace knit::actor::source::stm32;

int main()
{
    SPIParmas spi_params;
    spi_params.spi_bytes = 400;
    spi_params.spi_clock_hz = 3000000;
    spi_params.spi_dev_name = "/dev/spidev0.0";
    std::thread spi_loop_th, mag_loop_th;

    try
    {

        SPI spi(spi_params);
        std::vector<SPI::MarkedTube> tubes;
        // spi.emplace_tube({SensorName::RM3100}, tubes);
        spi.emplace_tube({SensorName::MPU9250_MAG}, tubes);
        spi_loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));

        // Rm3100 rm3100;
        MPU9250Mag mpu9250_mag;

        knit::actor::Tube<MagnetDataPtr> out;
        {
            // mag_loop_th = std::thread(&Rm3100::action_loop, &rm3100, std::ref(*tubes[0].second), std::ref(out));
            mag_loop_th = std::thread(&MPU9250Mag::action_loop, &mpu9250_mag, std::ref(*tubes[0].second), std::ref(out));
        }
        while (true)
        {
            auto res = out.get();
            std::cout << "timestamp: "
                      << res->timestamp
                      << ", xyz: "
                      << res->value.transpose()
                      << std::endl;
        }
    }
    catch (knit::actor::Exception e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
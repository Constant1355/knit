#include "../../sensors/clock/ds3231.hpp"
#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::sensor::clock;
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
        spi.emplace_tube({SensorName::DS3231}, tubes);
        spi_loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));

        // MPU9250 mpu9250;
        DS3231 ds3231;

        cloud_loop_th = std::thread(&DS3231::receive_loop, &ds3231, std::ref(*tubes[0].second));
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto ts = ds3231.latest();
            if (ts.has_value())
            {
                auto value = ts.value();
                std::cout << "stm32_on_second: " << value.stm32_on_second << std::endl;
                std::cout << "stm32_on_i2c   : " << value.stm32_on_i2c << std::endl;
                std::cout << "ds_seconds     : " << value.ds_seconds << std::endl;
                std::cout << "ds_date        : " << value.date.tm_hour
                          << " : " << value.date.tm_min
                          << " : " << value.date.tm_sec
                          << std::endl;
            }
            else
            {
                std::cout << "timeout and not data" << std::endl;
            }
        }
    }
    catch (knit::actor::Exception e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
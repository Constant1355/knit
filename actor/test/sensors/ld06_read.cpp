#include "../../sensors/ld06/cloud.hpp"
#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::sensor::ld06;
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
        spi.emplace_tube({SensorName::LD06}, tubes);
        spi_loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));

        CloudFrameParams cloud_params;
        cloud_params.filter_range = {0.2, 10};

        CloudFrame frame(cloud_params);

        knit::actor::Tube<Laser2dCloudPtr> out;
        cloud_loop_th = std::thread(&CloudFrame::action_loop, &frame, std::ref(*tubes[0].second), std::ref(out));

        while (true)
        {
            auto res = out.get();
            std::cout << "res: " << res->time_base << ", " << res->points.size() << std::endl;
            std::cout << "speed: " << frame.speed() << std::endl;
        }
    }
    catch (knit::actor::Exception e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
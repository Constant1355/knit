#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::source::stm32;

void spi_asychrono_loop(SPI &spi)
{
    std::vector<SPI::MarkedTube> tubes;
    spi.emplace_tube({SensorName::LD06, SensorName::MPU9250}, tubes);
    auto loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));
    while (loop_th.joinable())
    {
        auto msg = tubes[0].second->get();
        std::cout << "msg asychrono: "
                  << " name: "
                  << (int)msg->name
                  << "; tick: "
                  << std::dec
                  << msg->tick
                  << "; length: "
                  << msg->length
                  << std::endl;
    }
}

int main()
{
    SPIParmas spi_params;
    spi_params.spi_bytes = 400;
    spi_params.spi_clock_hz = 2000000;
    spi_params.spi_dev_name = "/dev/spidev0.1";

    SPI spi(spi_params);
    spi_asychrono_loop(spi);

    return 0;
}
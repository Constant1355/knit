#include "../../source/stm32/spi.hpp"
#include "../../sink/dump/dump.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::source::stm32;
using namespace knit::actor::sink::dump;

int main()
{
    SPIParmas spi_params;
    spi_params.spi_bytes = 400;
    spi_params.spi_clock_hz = 4000000;
    spi_params.spi_dev_name = "/dev/spidev0.1";

    SPI spi(spi_params);

    DumpParams dump_params;
    dump_params.dump_sensors.push_back(SensorName::LD06);
    dump_params.dump_sensors.push_back(SensorName::MPU9250);
    dump_params.dump_sensors.push_back(SensorName::YESENSE);

    dump_params.db = "test.db";

    Dump dump(dump_params);

    std::vector<SPI::MarkedTube> tubes;
    spi.emplace_tube(dump_params.dump_sensors, tubes);
    auto loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));
    auto dump_th = std::thread(&Dump::receive_loop, &dump, std::ref(*tubes[0].second));
    dump_th.join();
    loop_th.join();

    return 0;
}
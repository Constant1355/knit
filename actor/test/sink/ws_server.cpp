#include "../../sink/websocket/views.hpp"

using namespace knit::actor::source::stm32;
using namespace knit::actor::sink::websocket;

int main()
{
    SPIParmas spi_params;
    spi_params.spi_bytes = 400;
    spi_params.spi_clock_hz = 4000000;
    spi_params.spi_dev_name = "/dev/spidev0.0";

    SPI spi(spi_params);

    ViewParams view_params;
    view_params.timeout = std::chrono::seconds(60);

    view_params.dump_sensors.push_back(SensorName::LD06);
    view_params.dump_sensors.push_back(SensorName::MPU9250);
    view_params.dump_sensors.push_back(SensorName::YESENSE);
    view_params.dump_sensors.push_back(SensorName::DS3231);
    view_params.dump_sensors.push_back(SensorName::E108);
    view_params.dump_sensors.push_back(SensorName::RM3100);
    view_params.dump_sensors.push_back(SensorName::MPU9250_MAG);
    view_params.dump_sensors.push_back(SensorName::Battery_VC);
    view_params.dump_sensors.push_back(SensorName::Heds);

    std::vector<SPI::MarkedTube> tubes;
    spi.emplace_tube(view_params.dump_sensors, tubes);

    View view(view_params);
    auto loop_th = std::thread(&SPI::send_loop, &spi, std::ref(tubes));
    auto serve_th = std::thread(&View::receive_loop, &view, std::ref(*tubes[0].second));
    serve_th.join();
    loop_th.join();
}
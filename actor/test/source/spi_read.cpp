#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::source::stm32;

bool terminate = false;

void spi_asychrono_loop(SPI &spi)
{
    std::vector<SPI::MarkedTube> tubes;
    spi.emplace_tube({// SensorName::LD06, SensorName::MPU9250, SensorName::E108
                      SensorName::MPU9250_MAG},
                     tubes);
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

        // for (int idx = 0; idx < msg->length; idx += 2)
        // {
        //     float value = (float)(*(uint16_t *)(msg->load.data() + idx)) / 4096 * 3.3;
        //     std::cout << value * 6 << ", ";
        // }
        // for (int idx = 0; idx < msg->length; idx += 5)
        // {
        //     auto heading = *(uint8_t *)(msg->load.data() + idx);
        //     auto value = *(uint32_t *)(msg->load.data() + idx + 1);
        //     std::cout << (int)heading << ", " << value << "; ";
        // }

        // std::cout << std::endl;
    }
}

void spi_command_loop(SPI &spi)
{
    while (not terminate)
    {
        std::vector<uint8_t> params(4);
        *reinterpret_cast<uint32_t *>(params.data()) = 150;
        auto n = std::chrono::steady_clock::now();
        auto res = spi.command(1, params, 1000);
        auto elapse = std::chrono::steady_clock::now() - n;
        std::cout << "delay: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapse).count() << std::endl;
        if (res)
        {
            std::cout << "get response: " << res->size() << std::endl;
        }
        else
        {
            std::cout << "no response: " << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
int main()
{
    SPIParmas spi_params;
    spi_params.spi_bytes = 400;
    spi_params.spi_clock_hz = 2000000;
    spi_params.spi_dev_name = "/dev/spidev0.0";

    SPI spi(spi_params);
    auto command_th = std::thread(&spi_command_loop, std::ref(spi));
    spi_asychrono_loop(spi);
    terminate = true;
    command_th.join();

    return 0;
}
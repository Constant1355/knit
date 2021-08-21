#include "../../sensors/clock/ds3231.hpp"
#include "../../sensors/coordinate/mpu9250.hpp"
#include "../../sensors/coordinate/mpu9250_mag.hpp"
#include "../../sensors/coordinate/rm3100.hpp"
#include "../../sensors/coordinate/yesense.hpp"
#include "../../sensors/ld06/cloud.hpp"
#include "../../source/stm32/spi.hpp"
#include "../../common/exception.hpp"

using namespace knit::actor::sensor::clock;
using namespace knit::actor::source::stm32;
using namespace knit::actor::sensor::ld06;
using namespace knit::actor::sensor::coordinate;

template <typename ActorT, typename DataT>
void tube_th(ActorT &actor, knit::actor::Tube<SPIMessagePtr> &msg, std::function<void(knit::actor::Tube<DataT> &)> print);

void ld06_print(CloudFrame &frame, knit::actor::Tube<Laser2dCloudPtr> &out);
void mpu9250_print(MPU9250 &mpu9250, knit::actor::Tube<IMUDataPtr> &out);
void mpu9250_mag_print(MPU9250Mag &mpu9250_mag, knit::actor::Tube<MagnetDataPtr> &out);
void rm3100_print(Rm3100 &rm3100, knit::actor::Tube<MagnetDataPtr> &out);
void yesense_print(Yesense &yesense, knit::actor::Tube<IMUDataPtr> &out);

int main()
{
    try
    {
        std::vector<SPI::MarkedTube> tubes;
        SPIParmas spi_params;
        spi_params.spi_bytes = 400;
        spi_params.spi_clock_hz = 3000000;
        spi_params.spi_dev_name = "/dev/spidev0.1";

        SPI spi(spi_params);
        spi.emplace_tube({SensorName::DS3231}, tubes);
        spi.emplace_tube({SensorName::LD06}, tubes);
        spi.emplace_tube({SensorName::MPU9250}, tubes);
        spi.emplace_tube({SensorName::MPU9250_MAG}, tubes);
        spi.emplace_tube({SensorName::RM3100}, tubes);
        spi.emplace_tube({SensorName::YESENSE}, tubes);

        DS3231 ds3231;
        CloudFrameParams cloud_params;
        cloud_params.filter_range = {0.2, 10};
        CloudFrame frame(cloud_params);
        MPU9250 mpu9250;
        MPU9250Mag mpu9250_mag;
        Rm3100 rm3100;
        Yesense yesense;

        auto source_th = std::thread(&SPI::send_loop, std::ref(spi), std::ref(tubes));
        auto ds3231_th = std::thread(&DS3231::receive_loop, std::ref(ds3231), std::ref(*tubes[0].second));
        auto ld06_th = std::thread(&tube_th<CloudFrame, Laser2dCloudPtr>, std::ref(frame), std::ref(*tubes[1].second), std::bind(&ld06_print, std::ref(frame), std::placeholders::_1));
        auto mpu9250_th = std::thread(&tube_th<MPU9250, IMUDataPtr>, std::ref(mpu9250), std::ref(*tubes[2].second), std::bind(&mpu9250_print, std::ref(mpu9250), std::placeholders::_1));
        auto mpu9250_mag_th = std::thread(&tube_th<MPU9250Mag, MagnetDataPtr>, std::ref(mpu9250_mag), std::ref(*tubes[3].second), std::bind(&mpu9250_mag_print, std::ref(mpu9250_mag), std::placeholders::_1));
        auto rm3100_th = std::thread(&tube_th<Rm3100, MagnetDataPtr>, std::ref(rm3100), std::ref(*tubes[4].second), std::bind(&rm3100_print, std::ref(rm3100), std::placeholders::_1));
        auto yesense_th = std::thread(&tube_th<Yesense, IMUDataPtr>, std::ref(yesense), std::ref(*tubes[5].second), std::bind(&yesense_print, std::ref(yesense), std::placeholders::_1));

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            auto ts = ds3231.latest();
            if (ts.has_value())
            {
                auto date = ts.value();
                char dateBuff[1024];
                strftime(dateBuff, 1024, "%Y-%m-%d %H:%M:%S", &date.date);
                std::cout << "ds3231: " << date.ds_seconds << "; " << dateBuff << std::endl;
            }
        }
    }
    catch (knit::actor::Exception e)
    {
        std::cout << "main: " << e.what() << std::endl;
    }
    return 0;
}

template <typename ActorT, typename DataT>
void tube_th(ActorT &actor, knit::actor::Tube<SPIMessagePtr> &msg, std::function<void(knit::actor::Tube<DataT> &)> print)
{
    try
    {
        knit::actor::Tube<DataT> out;
        std::thread loop(&ActorT::action_loop, std::ref(actor), std::ref(msg), std::ref(out));
        while (true)
        {
            print(out);
        }
    }
    catch (knit::actor::Exception e)
    {
        std::cout << typeid(ActorT).name() << " exit tube_th: " << e.what() << std::endl;
    }
}

/////////////////////////////////////////

void ld06_print(CloudFrame &frame, knit::actor::Tube<Laser2dCloudPtr> &out)
{
    auto res = out.get();
    std::cout << "ld06 timebase: " << res->time_base << ", points: " << res->points.size() << ", motor speed: " << frame.speed() << std::endl;
}
void mpu9250_print(MPU9250 &mpu9250, knit::actor::Tube<IMUDataPtr> &out)
{
    auto res = out.get();
    // std::cout << "mpu9250 pulse: " << mpu9250.pulse.first << "; " << mpu9250.pulse.second << std::endl;
    std::cout << "mpu9250 counter: " << res->timestamp << "; " << res->count_inter << "; " << res->count_task << "; " << res->acc.transpose() << std::endl;
}
void mpu9250_mag_print(MPU9250Mag &mpu9250_mag, knit::actor::Tube<MagnetDataPtr> &out)
{
    auto res = out.get();
    std::cout << "mpu9250_mag value: " << res->value.transpose() << std::endl;
}
void rm3100_print(Rm3100 &rm3100, knit::actor::Tube<MagnetDataPtr> &out)
{
    auto res = out.get();
    std::cout << "rm3100 value: " << res->value.transpose() << std::endl;
}
void yesense_print(Yesense &yesense, knit::actor::Tube<IMUDataPtr> &out)
{
    auto res = out.get();
    std::cout << "yesense counter: " << res->timestamp << "; " << res->count_inter << "; " << res->count_task << "; " << res->acc.transpose() << std::endl;
    // std::cout << "yesense pulse: " << yesense.pulse.first << "; " << yesense.pulse.second << std::endl;
}

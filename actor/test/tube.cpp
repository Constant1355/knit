#include <iostream>
#include <thread>
#include "../common/queue.hpp"

int main()
{
    knit::actor::Tube<int> tube;
    std::thread set_value([&tube]()
                          {
                              std::this_thread::sleep_for(std::chrono::seconds(1));
                              tube.set_value(1);
                              tube.set_value(2);
                              tube.set_value(3);
                          });

    std::cout << "tube: " << tube.get() << std::endl;
    std::cout << "tube: " << tube.get() << std::endl;
    std::cout << "tube: " << tube.get() << std::endl;
    set_value.join();
    return 0;
}
#pragma once

#include <bitset>
#include <tuple>
#include <memory>
#include <vector>
#include "base.hpp"
#include "queue.hpp"
#include "exception.hpp"

namespace knit
{
    namespace actor
    {

        template <typename T, size_t N>
        class Source : public Base
        {
        public:
            using MarkedTube = std::pair<std::bitset<N>, TubePtr<T>>;
            virtual void send_loop(std::vector<MarkedTube> &out)
            {
                try
                {
                    set_status(RunStatus::Running);
                    while (get_status() != RunStatus::Terminated)
                    {
                        while (get_status() == RunStatus::Pause)
                        {
                            std::this_thread::yield();
                        }
                        send(out);
                    }
                }
                catch (knit::actor::Exception e)
                {
                    std::cout << "Source send loop: " << e.what() << std::endl;
                    exit(0);
                }
            }

            virtual void send(std::vector<MarkedTube> &out) = 0;
        };

        template <typename I, typename O>
        class Worker : public Base
        {
        public:
            virtual void action_loop(Tube<I> &in, Tube<O> &out)
            {
                try
                {
                    set_status(RunStatus::Running);
                    while (get_status() != RunStatus::Terminated)
                    {
                        while (get_status() == RunStatus::Pause)
                        {
                            std::this_thread::yield();
                        }
                        action(in, out);
                    }
                }

                catch (knit::actor::Exception e)
                {
                    std::cout << e.what() << std::endl;
                }
            }

            virtual void action(Tube<I> &in, Tube<O> &out) = 0;
            std::pair<double, size_t> pulse;
        };

        template <typename T>
        class Sink : public Base
        {
        public:
            virtual void receive_loop(Tube<T> &in)
            {
                try
                {
                    set_status(RunStatus::Running);
                    while (get_status() != RunStatus::Terminated)
                    {
                        while (get_status() == RunStatus::Pause)
                        {
                            std::this_thread::yield();
                        }
                        receive(in);
                    }
                }
                catch (knit::actor::Exception e)
                {
                    std::cout << "sink receive loop: " << e.what() << std::endl;
                    exit(0);
                }
            }

            virtual void receive(Tube<T> &in) = 0;
        };

    }
}
#pragma once

#include <future>
#include <tuple>
#include "base.hpp"

namespace knit
{
    namespace actor
    {
        template <typename T>
        class Tube
        {
        public:
            Tube()
            {
                promise_q_.push({});
                future_q_.push(promise_q_.back().get_future());
            }

            void set_value(const T &data)
            {
                std::lock_guard<std::mutex> lg(mtx_);
                promise_q_.back().set_value(data);
                promise_q_.push({});
                future_q_.push(promise_q_.back().get_future());
            }
            T get()
            {
                auto data = future_q_.front().get();
                std::lock_guard<std::mutex> lg(mtx_);
                future_q_.pop();
                promise_q_.pop();
                return data;
            }

        private:
            std::mutex mtx_;
            std::queue<std::promise<T>> promise_q_;
            std::queue<std::future<T>> future_q_;
        };

        template <typename T>
        using TubePtr = std::shared_ptr<Tube<T>>;

    }

}
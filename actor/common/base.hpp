#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <future>

namespace knit
{
    namespace actor
    {

        enum class RunStatus
        {
            Terminated,
            Running,
            Pause
        };
        class Base
        {
        public:
            Base() : status_(RunStatus::Terminated) {}

            virtual ~Base() { set_status(RunStatus::Terminated); }

            RunStatus get_status()
            {
                return status_.load();
            }

            void set_status(const RunStatus &status)
            {
                status_.store(status);
            }

        private:
            std::atomic<RunStatus> status_;
        };

    }
}

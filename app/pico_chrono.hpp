#if !defined(PICO_CHRONO_HPP)
#define PICO_CHRONO_HPP

#include <chrono>

#include "pico/time.h"

namespace pico
{
    namespace chrono
    {
        std::chrono::microseconds sys_time_now() noexcept
        {
            return std::chrono::microseconds{time_us_64()};
        }

        void this_core_sleep(std::chrono::microseconds duration)
        {
            sleep_ms(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    }
}

#endif
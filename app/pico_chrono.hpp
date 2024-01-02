#if !defined(PICO_CHRONO_HPP)
#define PICO_CHRONO_HPP

#include <chrono>

#include "pico/time.h"

namespace pico
{
    namespace chrono
    {
        std::chrono::microseconds sys_time_now() noexcept;
        void this_core_sleep(std::chrono::microseconds duration) noexcept;
    }
}

#endif
#include "pico_panic.hpp"

#include "pico_chrono.hpp"
#include "pico/stdlib.h"
#include "pico/printf.h"

using namespace std::chrono_literals;

namespace pico
{
    namespace panic
    {
        void print_forever(char character)
        {
            for (;;)
            {
                printf("0x%02X\n", character);
                pico::chrono::this_core_sleep(1s);
            }
        }
        void loop_forever()
        {
            for (;;)
            {
                tight_loop_contents();
            }
        }
    }
}
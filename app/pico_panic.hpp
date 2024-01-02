#if !defined(PICO_PANIC_HPP)
#define PICO_PANIC_HPP

namespace pico
{
    namespace panic
    {
        void print_forever(char character);
        void loop_forever();
    }
}

#endif
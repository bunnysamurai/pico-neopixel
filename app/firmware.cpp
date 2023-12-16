#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include <cstdio>
#include "ws2812/ws2812.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include "constexpr_math.hpp"
#include "led_driver.hpp"

static constexpr auto NEOPIXEL_PIN{2};
static constexpr auto NEOPIXEL_LED_COUNT{24};

static const PIO PIO_INDEX{pio0};
static constexpr auto PIO_STATE_MACHINE{0};

int main()
{
    LED_Driver led_driver(PICO_DEFAULT_LED_PIN);

    pico_ws2812::PIO_NeoPixel_Driver driver(PIO_INDEX, PIO_STATE_MACHINE, NEOPIXEL_PIN);
    pico_ws2812::WRGB_Driver pixel_driver{driver};
    pico_ws2812::WRGB_Pattern_Driver pattern_driver{pixel_driver, NEOPIXEL_LED_COUNT};

    auto pattern_generator{[](uint index)
                           {
                               return [=](uint)
                               {
                                   const auto pixel_value{SINE_TABLE<128>[index & SINE_TABLE_MASK(SINE_TABLE<128>)]};
                                   return pico_ws2812::WRGB{.white{pixel_value}, .red{0}, .green{0}, .blue{0}};
                               };
                           }};

    uint index{0};
    for (;;)
    {
        pattern_driver.put_pattern(pattern_generator(index));
        auto pixel_value{SINE_TABLE<128>[index & SINE_TABLE_MASK(SINE_TABLE<128>)]};
        led_driver.put_pixel(pixel_value);
        index++;
        sleep_ms(50);
    }
}

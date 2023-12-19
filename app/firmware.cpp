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

static constexpr auto SINE_TABLE_LENGTH{1024};
static constexpr auto GAMMA_CURVE_LENGTH{256};

int main()
{
    LED_Driver led_driver(PICO_DEFAULT_LED_PIN);

    pico_ws2812::PIO_NeoPixel_Driver driver(PIO_INDEX, PIO_STATE_MACHINE, NEOPIXEL_PIN);
    pico_ws2812::WRGB_Driver pixel_driver{driver};
    pico_ws2812::WRGB_Pattern_Driver pattern_driver{pixel_driver, NEOPIXEL_LED_COUNT};

    auto pattern_generator{[](uint index)
                           {
                               return [=](uint pixel_index)
                               {
                                   const auto sine_index{index + pixel_index * SINE_TABLE_LENGTH / NEOPIXEL_LED_COUNT};
                                   const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[sine_index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
                                   const auto gamma_pixel_value{GAMMA_CURVE<GAMMA_CURVE_LENGTH>[pixel_value]};
                                   // full 255 is too bright, let's scale down by a factor of 8 
                                   return pico_ws2812::WRGB{.white{static_cast<uint8_t>(gamma_pixel_value >> 4)}, .red{0}, .green{0}, .blue{0}};
                               };
                           }};

    uint index{SINE_TABLE_LENGTH / 2};
    for (;;)
    {
        pattern_driver.put_pattern(pattern_generator(index));
        const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
        led_driver.put_pixel(GAMMA_CURVE<GAMMA_CURVE_LENGTH>[pixel_value]);
        index++;
        sleep_ms(5);
    }
}

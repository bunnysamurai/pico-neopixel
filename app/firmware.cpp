#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include <cstdio>
#include "ws2812/ws2812.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

static constexpr auto NEOPIXEL_PIN{2};
static constexpr auto NEOPIXEL_LED_COUNT{24};

static const PIO PIO_INDEX{pio0};
static constexpr auto PIO_STATE_MACHINE{0};

// FIXME THIS IS TERRIBLE, AS WE CAN RUN INTO NUMERICAL ISSUES VERY QUICKLY
inline constexpr double integral_pow(double value, uint exp)
{
    double rv{1};
    for (uint ii{0}; ii < exp; ++ii)
    {
        rv *= value;
    }
    return rv;
}
static_assert(integral_pow(2.0, 4) == 2.0 * 2.0 * 2.0 * 2.0);

inline constexpr uint64_t factorial(uint64_t value)
{
    uint64_t rv{1};
    for (uint64_t ii = 0; ii < value; ++ii)
    {
        rv *= ii + 1;
    }
    return rv;
}
static_assert(factorial(1) == 1);
static_assert(factorial(2) == 2);
static_assert(factorial(3) == 1 * 2 * 3);
static_assert(factorial(5) == 120);

inline constexpr double sine_power_series(double radian, size_t terms)
{
    double rv{};
    for (size_t n = 0; n < terms; ++n)
    {
        const double numer{integral_pow(radian, 2 * n + 1)};
        const double denom{static_cast<double>(factorial(2 * n + 1))};
        const double factor{integral_pow(-1.0, n)};
        rv += (numer / denom) * factor;
    }
    return rv;
}
static_assert(sine_power_series(0.0, 5) == 0);
static_assert(sine_power_series(3.141592653589793, 5) > 0);
static_assert(sine_power_series(270 * 3.141592653589793 / 180, 5) < 0);
static_assert(sine_power_series(2 * 3.141592653589793, 5) > 0);

template <size_t N>
    requires(std::popcount(N) == 1)
static constexpr auto SINE_TABLE{
    []()
    {
        constexpr auto POWER_SERIES_TERMS{11};
        // https://math.stackexchange.com/questions/2853310/how-to-calculate-the-sine-manually-without-any-rules-calculator-or-anything-el#2853320
        // using the power-series method
        std::array<uint8_t, N> rv{};
        for (size_t ii = 0; ii < std::size(rv); ++ii)
        {
            const double radian{static_cast<double>(ii) / std::size(rv) * 2 * 3.141592653589793};
            const double value{(sine_power_series(radian, POWER_SERIES_TERMS) + 1.0) / 2.0};
            rv[ii] = static_cast<uint16_t>(value * std::numeric_limits<uint8_t>::max()) & 0xFFU;
        }
        return rv;
    }()};

inline constexpr auto SINE_TABLE_MASK(const auto &sine_table) { return std::size(sine_table) - 1; }

class LED_Driver
{
public:
    LED_Driver(int pin_number) : m_pin{pin_number}, m_wrap{0xFFFFU}
    {
        gpio_set_function(m_pin, GPIO_FUNC_PWM);

        pwm_config cfg{pwm_get_default_config()};
        pwm_config_set_wrap(&cfg, m_wrap);

        const auto slice{pwm_gpio_to_slice_num(m_pin)};
        const auto channel{pwm_gpio_to_channel(m_pin)};
        pwm_set_chan_level(slice, channel, 0);
        pwm_init(slice, &cfg, true);
    }
    void put_pixel(uint32_t value)
    {
        const auto slice{pwm_gpio_to_slice_num(m_pin)};
        const auto channel{pwm_gpio_to_channel(m_pin)};
        pwm_set_chan_level(slice, channel, to_pwm_level(value & 0xFFU));
    }
    void set_rgb_mode()
    { /* do nothing*/
    }
    void set_wrgb_mode()
    { /* do nothing*/
    }

private:
    int m_pin;
    uint16_t m_wrap;

    [[nodiscard]] constexpr uint16_t to_pwm_level(uint8_t level)
    {
        return level * m_wrap / std::numeric_limits<uint8_t>::max();
    }
};

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

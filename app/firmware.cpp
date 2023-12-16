#include "hardware/pio.h"
#include "pico/time.h"
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
        const double factor{(n & 0x1U) == 0 ? 1.0 : -1.0};
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
        constexpr auto POWER_SERIES_TERMS{5};
        // https://math.stackexchange.com/questions/2853310/how-to-calculate-the-sine-manually-without-any-rules-calculator-or-anything-el#2853320
        // using the power-series method
        std::array<uint8_t, N> rv{};
        for (size_t ii = 0; ii < std::size(rv); ++ii)
        {
            const double radian{static_cast<double>((2 * 3.141592653589793) / std::size(rv) * ii)};
            const auto value{sine_power_series(radian, POWER_SERIES_TERMS)};
            rv[ii] = static_cast<uint8_t>((value + 1) / 2) + std::numeric_limits<uint8_t>::max();
        }
        return rv;
    }()};

inline constexpr auto SINE_TABLE_MASK(const auto &sine_table) { return static_cast<uint8_t>(~(std::size(sine_table) - 1)); }

int main()
{
    pico_ws2812::PIO_NeoPixel_Driver driver(PIO_INDEX, PIO_STATE_MACHINE, NEOPIXEL_PIN);
    pico_ws2812::WRGB_Driver pixel_driver{driver};
    pico_ws2812::WRGB_Pattern_Driver pattern_driver{pixel_driver, NEOPIXEL_LED_COUNT};

    auto pattern_generator{[index = uint{0}](uint) mutable
                           {
                               const auto pixel_value{SINE_TABLE<1024>[index++ & SINE_TABLE_MASK(SINE_TABLE<1024>)]};
                               return pico_ws2812::WRGB{.white{pixel_value}, .red{0}, .green{0}, .blue{0}};
                           }};

    for (;;)
    {
        pattern_driver.put_pattern(pattern_generator);
        sleep_ms(5);
    }
}

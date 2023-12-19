#if !defined(CONSTEXPR_MATH_HPP)
#define CONSTEXPR_MATH_HPP

#include <cstdint>
#include <bit>
#include <cstddef>
#include <array>

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
    for (uint64_t ii{0}; ii < value; ++ii)
    {
        rv *= ii + 1;
    }
    return rv;
}
static_assert(factorial(1) == 1);
static_assert(factorial(2) == 2);
static_assert(factorial(3) == 1 * 2 * 3);
static_assert(factorial(5) == 120);

/* approximate sin(radian) using the Talyor-series expansion */
inline constexpr double sine_power_series(double radian, size_t terms)
{
    double rv{};
    for (size_t n{0}; n < terms; ++n)
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

/* approximate e^(exp) using Taylor-series expansion */
inline constexpr double exponential_power_series(double exp, size_t terms)
{
    double rv{};
    for (size_t n{0}; n < terms; ++n)
    {
        rv += integral_pow(exp, n) / factorial(n);
    }
    return rv;
}
static_assert(exponential_power_series(1, 11) > 2.71828);
static_assert(exponential_power_series(1, 11) - 2.71828 < 0.001);
static_assert(exponential_power_series(1.5, 11) > 4.481);
static_assert(exponential_power_series(1.5, 11) - 4.481 < 0.001);
static_assert(exponential_power_series(0.1, 11) > 1.105);
static_assert(exponential_power_series(0.1, 11) - 1.105 < 0.001);

/* approximate ln(val) using Taylor-series expansion */
inline constexpr double logarithm_power_series(double val, size_t terms)
{
    double rv{};
    for (size_t n{0}; n < terms; ++n)
    {
        const auto exp_val{exponential_power_series(rv, terms)};
        rv += 2 * (val - exp_val) / (val + exp_val);
    }
    return rv;
}
static_assert(logarithm_power_series(2.718281828450945, 11) > 1);
static_assert(logarithm_power_series(2.718281828450945, 11) - 1 < 0.001);

template <size_t N>
    requires(std::popcount(N) == 1)
inline constexpr auto SINE_TABLE{
    []()
    {
        constexpr auto POWER_SERIES_TERMS{11};
        // https://math.stackexchange.com/questions/2853310/how-to-calculate-the-sine-manually-without-any-rules-calculator-or-anything-el#2853320
        // using the power-series method
        std::array<uint8_t, N> rv{};
        for (size_t ii{0}; ii < std::size(rv); ++ii)
        {
            const double radian{static_cast<double>(ii) / std::size(rv) * 2 * 3.141592653589793};
            const double value{(sine_power_series(radian, POWER_SERIES_TERMS) + 1.0) / 2.0};
            rv[ii] = static_cast<uint16_t>(value * std::numeric_limits<uint8_t>::max()) & 0xFFU;
        }
        return rv;
    }()};

inline constexpr auto SINE_TABLE_MASK(const auto &sine_table) { return std::size(sine_table) - 1; }

template <size_t N>
    requires(std::popcount(N) == 1)
inline constexpr auto GAMMA_CURVE{
    // An sRGB gamma correction
    []()
    {
        /* We transfer from linear to compressed representation
         * Given Input is on the range [0,1]
         *        /  12.92 * Input,                     Input <= 0.0031308
         *  Out = |
         *        \  1.055 * (Input)^(1/2.4) - 0.055,   Input > 0.0031308
         */
        // the hard part is the fractional power
        // X ^ (1/2.4) can be done using logarithms!
        //      ln(x^n) == n * ln(x)
        //      exp(ln(y)) == y
        constexpr auto POWER_SERIES_TERMS{21};
        constexpr auto LOWER_THRESHOLD{0.0031308};
        constexpr auto LOWER_FACTOR{12.92};
        constexpr auto UPPER_FACTOR{1.055};
        constexpr auto UPPER_EXP{1 / 2.4};
        constexpr auto UPPER_OFFSET{-0.055};

        constexpr auto lower_fn{[](auto val)
                                {
                                    return LOWER_FACTOR * val;
                                }};
        constexpr auto upper_fn{[](auto val)
                                {
                                    const auto log_val{UPPER_EXP * logarithm_power_series(val, POWER_SERIES_TERMS)};
                                    return UPPER_FACTOR * exponential_power_series(log_val, POWER_SERIES_TERMS) + UPPER_OFFSET;
                                }};
        std::array<uint8_t, N> rv{};
        for (size_t ii{0}; ii < N; ++ii)
        {
            const double linear{static_cast<double>(ii) / (N - 1)};
            const double gamma{linear <= LOWER_THRESHOLD ? lower_fn(linear) : upper_fn(linear)};
            rv[ii] = static_cast<uint8_t>(gamma * 255 > 255 ? 255 : gamma * 255);
        }
        return rv;
    }()};

#endif
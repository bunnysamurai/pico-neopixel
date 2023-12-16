#include "hardware/pio.h"
#include "pico/time.h"
#include "ws2812/ws2812.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

static constexpr auto NEOPIXEL_PIN{ 2 };
static constexpr auto NEOPIXEL_LED_COUNT{ 24 };

static const PIO PIO_INDEX{ pio0 };
static constexpr auto PIO_STATE_MACHINE{ 0 };

template< size_t N >
    requires( std::popcount( N ) == 1 )
static constexpr auto SINE_TABLE{
    []() {
        // FIXME THIS IS TERRIBLE, AS WE CAN RUN INTO NUMERICAL ISSUES VERY QUICKLY
        auto&& integral_pow{ []( double value, int exp ) {
            double rv{ 1 };
            for( int ii{ 0 }; ii < exp; ++ii )
            {
                rv *= value;
            }
            return rv;
        } };
        auto&& factoral{ []( int value ) {
            double rv{ 1 };
            for( int ii = 0; ii < value; ++ii )
            {
                rv *= ii + 1;
            }
            return rv;
        } };

        auto&& power_series{ [ = ]( double radian, size_t terms ) {
            double rv{};
            for( size_t n = 0; n < terms; ++n )
            {
                const double numer{ integral_pow( radian, 2 * n + 1 ) };
                const double denom{ factoral( 2 * n + 1 ) };
                const double factor{ ( n & 0x1U ) == 0 ? 1.0 : -1.0 };
                rv += numer / denom * factor;
            }
            return rv;
        } };
        // https://math.stackexchange.com/questions/2853310/how-to-calculate-the-sine-manually-without-any-rules-calculator-or-anything-el#2853320
        // using the power-series method
        std::array< uint8_t, N > rv{};
        for( size_t ii = 0; ii < std::size( rv ); ++ii )
        {
            const double radian{ ( 2 * 3.141592653589793 ) / std::size( rv ) * ii };
            rv[ ii ] = power_series( radian, 1 ) * 128 + 127;
        }
        return rv;
    }()
};

inline constexpr auto SINE_TABLE_MASK( const auto& sine_table ) { return static_cast< uint8_t >( ~( std::size( sine_table ) - 1 ) ); }

int main()
{
    pico_ws2812::WRGB_String_Driver drv( PIO_INDEX, PIO_STATE_MACHINE, NEOPIXEL_PIN, NEOPIXEL_LED_COUNT );

    uint8_t index{ 0 };
    for( ;; )
    {
        drv.put_pixel( SINE_TABLE< 128 >[ index++ & SINE_TABLE_MASK( SINE_TABLE< 128 > ) ] );
        sleep_ms( 10 );
    }
}

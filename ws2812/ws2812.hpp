#if !defined( WS2812_HPP )
#define WS2812_HPP

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdlib>

#include "generated/ws2812.pio.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include <cstdint>
#include <utility>

namespace pico_ws2812
{

    class RGB_String_Driver
    {
    public:
        using Callable_T = uint32_t ( * )( uint pixel_index );

        explicit RGB_String_Driver( PIO pio_index, int state_machine_index, int gpio_pin, size_t number_of_pixels ) noexcept :
            m_pio{ pio_index }, m_state_machine{ state_machine_index }, m_number_of_pixels{ number_of_pixels }
        {
            uint offset = pio_add_program( m_pio, &ws2812_program );
            ws2812_program_init( m_pio, m_state_machine, offset, gpio_pin, 800000, false );
        }

        void put_pattern( Callable_T functor )
        {
            for( uint ii = 0; ii < m_number_of_pixels; ++ii )
            {
                put_pixel( functor( ii ) );
            }
        }

        void put_pixel( uint8_t r, uint8_t g, uint8_t b )
        {
            pio_sm_put_blocking( m_pio, m_state_machine, rgb_u32( r, g, b ) );
        }

    protected:
        PIO m_pio;
        int m_state_machine;
        size_t m_number_of_pixels;

        void put_pixel( uint32_t pixel_grb )
        {
            pio_sm_put_blocking( m_pio, m_state_machine, pixel_grb );
        }

        inline uint32_t rgb_u32( uint8_t r, uint8_t g, uint8_t b )
        {
            return ( static_cast< uint32_t >( r ) << 16 ) |
                   ( static_cast< uint32_t >( g ) << 24 ) |
                   ( static_cast< uint32_t >( b ) << 8 );
        }
    };

    class WRGB_String_Driver : public RGB_String_Driver
    {
    public:
        using RGB_String_Driver::Callable_T;

        explicit WRGB_String_Driver( PIO pio_index, int state_machine_index, int gpio_pin, size_t number_of_pixels ) noexcept :
            RGB_String_Driver( pio_index, state_machine_index, gpio_pin, number_of_pixels )
        {
            uint offset = pio_add_program( m_pio, &ws2812_program );
            ws2812_program_init( m_pio, m_state_machine, offset, gpio_pin, 800000, true );
        }

        void put_pixel( uint8_t w, uint8_t r, uint8_t g, uint8_t b )
        {
            pio_sm_put_blocking( m_pio, m_state_machine, wrgb_u32( w, r, g, b ) );
        }
        void put_pixel( uint8_t w )
        {
            pio_sm_put_blocking( m_pio, m_state_machine, wrgb_u32( w, 0, 0, 0 ) );
        }

    private:
        inline uint32_t wrgb_u32( uint8_t w, uint8_t r, uint8_t g, uint8_t b )
        {
            return ( static_cast< uint32_t >( r ) << 24 ) |
                   ( static_cast< uint32_t >( g ) << 16 ) |
                   ( static_cast< uint32_t >( b ) << 8 ) |
                   static_cast< uint32_t >( w );
        }
    };
}

#endif
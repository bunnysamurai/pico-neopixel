#if !defined(WS2812_HPP)
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
    template <class T>
    concept string_interface = requires(T a, uint32_t pixel) {
        a.put_pixel(pixel);
        a.set_rgb_mode();
        a.set_wrgb_mode();
    };

    struct RGB
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    template <class T>
    concept rgb_interface = requires(T a, RGB pixel) {
        a.put_pixel(pixel);
    };

    struct WRGB
    {
        uint8_t white;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    template <class T>
    concept wrgb_interface = requires(T a, WRGB pixel) {
        a.put_pixel(pixel);
    };

    class PIO_NeoPixel_Driver final
    {
    public:
        explicit PIO_NeoPixel_Driver(PIO pio_index, int state_machine_index, int gpio_pin) noexcept : m_pio{pio_index}, m_state_machine{state_machine_index}, m_pin{gpio_pin}
        {
        }

        void set_rgb_mode() noexcept
        {
            uint offset = pio_add_program(m_pio, &ws2812_program);
            ws2812_program_init(m_pio, m_state_machine, offset, m_pin, 800000, false);
        }
        void set_wrgb_mode() noexcept
        {
            uint offset = pio_add_program(m_pio, &ws2812_program);
            ws2812_program_init(m_pio, m_state_machine, offset, m_pin, 800000, true);
        }

        void put_pixel(uint32_t pixel_value) noexcept
        {
            pio_sm_put_blocking(m_pio, m_state_machine, pixel_value);
        }

    private:
        PIO m_pio;
        int m_state_machine;
        int m_pin;
    };

    template <string_interface Driver>
    class RGB_Driver final
    {
    public:
        constexpr explicit RGB_Driver(Driver &drv) noexcept : m_drv{drv}
        {
            m_drv.set_rgb_mode();
        }

        constexpr void put_pixel(RGB pixel) noexcept
        {
            m_drv.put_pixel(rgb_u32(pixel));
        }

    protected:
        Driver &m_drv;

        [[nodiscard]] static constexpr uint32_t rgb_u32(RGB pixel) noexcept
        {
            return (static_cast<uint32_t>(pixel.red) << 16) |
                   (static_cast<uint32_t>(pixel.green) << 24) |
                   (static_cast<uint32_t>(pixel.blue) << 8);
        }
    };

    template <string_interface Driver>
    class WRGB_Driver final
    {
    public:
        constexpr explicit WRGB_Driver(Driver &drv) noexcept : m_drv{drv}
        {
            m_drv.set_wrgb_mode();
        }

        constexpr void put_pixel(WRGB pixel) noexcept
        {
            m_drv.put_pixel(wrgb_u32(pixel));
        }
        constexpr void put_pixel(uint8_t w) noexcept
        {
            m_drv.put_pixel(wrgb_u32(WRGB{.white{w}, .red{0}, .green{0}, .blue{0}}));
        }

    private:
        Driver &m_drv;

        [[nodiscard]] static constexpr uint32_t wrgb_u32(WRGB pixel) noexcept
        {
            return (static_cast<uint32_t>(pixel.green) << 24) |
                   (static_cast<uint32_t>(pixel.red) << 16) |
                   (static_cast<uint32_t>(pixel.blue) << 8) |
                   static_cast<uint32_t>(pixel.white);
        }
    };

    template <rgb_interface Driver>
    class RGB_Pattern_Driver final
    {
    public:
        RGB_Pattern_Driver(Driver &drv, uint number_of_pixels) noexcept : m_drv{drv}, m_number_of_pixels{number_of_pixels} {}

        template <class Callable>
        constexpr void put_pattern(Callable pattern_generator) noexcept
        {
            for (uint ii = 0; ii < m_number_of_pixels; ++ii)
            {
                m_drv.put_pixel(pattern_generator(ii));
            }
        }

    private:
        Driver &m_drv;
        uint m_number_of_pixels;
    };

    template <wrgb_interface Driver>
    class WRGB_Pattern_Driver final
    {
    public:
        WRGB_Pattern_Driver(Driver &drv, uint number_of_pixels) noexcept : m_drv{drv}, m_number_of_pixels{number_of_pixels} {}

        template <class Callable>
        constexpr void put_pattern(Callable pattern_generator) noexcept
        {
            for (uint ii = 0; ii < m_number_of_pixels; ++ii)
            {
                m_drv.put_pixel(pattern_generator(ii));
            }
        }

    private:
        Driver &m_drv;
        uint m_number_of_pixels;
    };
}

#endif
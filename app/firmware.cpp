#if 1
#include <cstring>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/stdio.h"
#include "pico/printf.h"

#include "Command.hpp"
#include "led_driver.hpp"
#include "pico_chrono.hpp"
#include "Ring_Buffer.hpp"
#include "Input_State_Machine.hpp"

using namespace std::chrono_literals;

namespace
{
    void blink_forever(std::chrono::milliseconds duration)
    {

        LED_Driver led_driver(PICO_DEFAULT_LED_PIN);
        for (;;)
        {
            led_driver.put_pixel(0xFF >> 4);
            pico::chrono::this_core_sleep(duration);
            led_driver.put_pixel(0x00);
            pico::chrono::this_core_sleep(duration);
        }
    }

    void wait_for_user_sync()
    {
        static constexpr auto TIMEOUT_US{1s};
        const auto give_up_start{pico::chrono::sys_time_now()};
        for (;;)
        {
            const auto start{pico::chrono::sys_time_now()};
            printf("Send any character to synchronize\n");
            int c{getchar_timeout_us(0)};
            if (c != PICO_ERROR_TIMEOUT && c != '\0')
            {
                break;
            }
            const auto duration_passed{pico::chrono::sys_time_now() - start};
            pico::chrono::this_core_sleep(TIMEOUT_US - duration_passed);
        }
    }

}

class PicoLogger
{
public:
    template <class... Args>
    void print(std::string_view str, Args &&...vals)
    {
        static constexpr auto MAX_LEN{80};
        std::array<char, MAX_LEN + 1> null_term{};
        if (std::size(str) > MAX_LEN)
        {
            printf("print_error_fail");
        }
        memcpy(std::data(null_term), std::data(str), std::size(str));
        printf(std::data(null_term), std::forward<Args>(vals)...);
    }
    template <class... Args>
    void print_error(std::string_view str, Args &&...vals)
    {
        static constexpr auto MAX_LEN{32};
        std::array<char, MAX_LEN + 1 + 4> null_term{};
        null_term[0] = '[';
        null_term[1] = 'E';
        null_term[2] = ']';
        null_term[3] = ' ';
        if (std::size(str) > MAX_LEN)
        {
            printf("print_error_fail");
        }
        memcpy(std::next(std::data(null_term), 4), std::data(str), std::size(str));
        printf(std::data(null_term), std::forward<Args>(vals)...);
    }

private:
};

template <class... Args>
void print(PicoLogger &dev, std::string_view str, Args &&...vals)
{
    dev.print(str, std::forward<Args>(vals)...);
}

template <class... Args>
void print_error(PicoLogger &dev, std::string_view str, Args &&...vals)
{
    print(dev, "[!] ");
    print(dev, str, std::forward<Args>(vals)...);
}

template <class... Args>
void print_info(PicoLogger &dev, std::string_view str, Args &&...vals)
{
    print(dev, "[*] ");
    print(dev, str, std::forward<Args>(vals)...);
}

template <class... Args>
void print_debug(PicoLogger &dev, std::string_view str, Args &&...vals)
{
    print(dev, "[@] ");
    print(dev, str, std::forward<Args>(vals)...);
}

static constexpr auto WARNING_BLINK_DURATION{100ms};
static constexpr auto INFO_BLINK_DURATION{2s};

static PicoLogger stdlogger;
static constexpr auto MAX_LINE_LENGTH_PER_COMMAND_INVOCATION{40};
static PicoLineProvider<MAX_LINE_LENGTH_PER_COMMAND_INVOCATION> line_provider;
static SM command_state_machine{line_provider, stdlogger};
static Command_SM command_runner{"[Meven5000]$ ", command_state_machine, stdlogger};

int main()
{
    if (!stdio_init_all())
    {
        blink_forever(WARNING_BLINK_DURATION);
    }

    wait_for_user_sync();

    // the input state machine will read in characters from input and stuff them in the command queue when ready
    // the command state machine processes any commnds in the queue
    printf("[Meven5000]$ ");
    for (;;)
    {
        line_provider.update();
        command_state_machine.update();
        command_runner.update();
    }
}

#else // old way

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
#include "Ring_Buffer.hpp"

static constexpr auto NEOPIXEL_PIN{2};
static constexpr auto NEOPIXEL_LED_COUNT{24};

static const PIO PIO_INDEX{pio0};
static constexpr auto PIO_STATE_MACHINE{0};

static constexpr auto SINE_TABLE_LENGTH{1024};
static constexpr auto SRGB_GAMMA_CURVE_LENGTH{256};

int main()
{
    stdio_init_all();

    LED_Driver led_driver(PICO_DEFAULT_LED_PIN);

    pico_ws2812::PIO_NeoPixel_Driver driver(PIO_INDEX, PIO_STATE_MACHINE, NEOPIXEL_PIN);
    pico_ws2812::WRGB_Driver pixel_driver{driver};
    pico_ws2812::WRGB_Pattern_Driver pattern_driver{pixel_driver, NEOPIXEL_LED_COUNT};

    auto test_pattern_generator{[](uint index)
                                {
                                    return [=](uint pixel_index)
                                    {
                                        // full 255 is too bright, let's scale down by a factor of 8
                                        if (pixel_index == 0)
                                        {
                                            const auto sine_index{index};
                                            const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[sine_index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
                                            const auto gamma_pixel_value{SRGB_GAMMA_CURVE<SRGB_GAMMA_CURVE_LENGTH>[pixel_value]};
                                            return pico_ws2812::WRGB{.white{static_cast<uint8_t>(gamma_pixel_value >> 4)}, .red{0}, .green{0}, .blue{0}};
                                        }
                                        else if (pixel_index == 2)
                                        {
                                            const auto sine_index{index};
                                            const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[sine_index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
                                            return pico_ws2812::WRGB{.white{static_cast<uint8_t>(pixel_value >> 4)}, .red{0}, .green{0}, .blue{0}};
                                        }
                                        else
                                        {
                                            return pico_ws2812::WRGB{.white{0}, .red{0}, .green{0}, .blue{0}};
                                        }
                                    };
                                }};
    auto pattern_generator{[](uint index)
                           {
                               return [=](uint pixel_index)
                               {
                                   const auto sine_index{index + pixel_index * SINE_TABLE_LENGTH / NEOPIXEL_LED_COUNT};
                                   const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[sine_index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
                                   const auto gamma_pixel_value{SRGB_GAMMA_CURVE<SRGB_GAMMA_CURVE_LENGTH>[pixel_value]};
                                   // full 255 is too bright, let's scale down by a factor of 8
                                   const auto adj{(gamma_pixel_value >> 4) + 1};
                                   return pico_ws2812::WRGB{.white{static_cast<uint8_t>(adj)}, .red{0}, .green{0}, .blue{0}};
                               };
                           }};
    auto pattern_generator_2{[](uint index)
                             {
                                 return [=](uint)
                                 {
                                     const auto sine_index{index};
                                     const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[sine_index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
                                     const auto gamma_pixel_value{SRGB_GAMMA_CURVE<SRGB_GAMMA_CURVE_LENGTH>[pixel_value]};
                                     // full 255 is too bright, let's scale down by a factor of 8
                                     const auto adj{(gamma_pixel_value >> 4) + 1};
                                     return pico_ws2812::WRGB{.white{static_cast<uint8_t>(adj)}, .red{0}, .green{0}, .blue{0}};
                                 };
                             }};

    uint64_t now_us{time_us_64()};
    uint index{SINE_TABLE_LENGTH / 2};
    for (;;)
    {
        pattern_driver.put_pattern(pattern_generator_2(index));
        const auto pixel_value{SINE_TABLE<SINE_TABLE_LENGTH>[index & SINE_TABLE_MASK(SINE_TABLE<SINE_TABLE_LENGTH>)]};
        led_driver.put_pixel(srgb_gamma_curve<srgb_gamma_curve_length>[pixel_value] >> 4);
        index++;
        sleep_ms(5);
    }
}

#endif
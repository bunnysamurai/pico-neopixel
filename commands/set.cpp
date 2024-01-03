#include "app/Command.hpp"

#include "pico/printf.h"

#include "ws2812/ws2812.hpp"

#include <utility>
#include <optional>
#include <charconv>

static constexpr auto NEOPIXEL_PIN{2};
static constexpr auto NEOPIXEL_LED_COUNT{24};

static const PIO PIO_INDEX{pio0};
static constexpr auto PIO_STATE_MACHINE{0};

static pico_ws2812::PIO_NeoPixel_Driver driver(PIO_INDEX, PIO_STATE_MACHINE, NEOPIXEL_PIN);
static pico_ws2812::WRGB_Driver pixel_driver{driver};

static std::array<pico_ws2812::WRGB, NEOPIXEL_LED_COUNT> pixel_buffer;

static void put_pixel_buffer()
{
    for (const auto &c : pixel_buffer)
    {
        pixel_driver.put_pixel(c);
    }
}

static void set_pixel_in_buffer(size_t index, pico_ws2812::WRGB new_value)
{
    pixel_buffer[index] = new_value;
}

static void fill_pixel_buffer(pico_ws2812::WRGB new_value)
{
    std::fill(std::begin(pixel_buffer), std::end(pixel_buffer), new_value);
}

[[nodiscard]] constexpr auto check_equality(const auto &str, std::string_view arg) noexcept
{
    if (std::size(str) != std::size(arg))
    {
        return false;
    }
    return std::transform_reduce(
        std::begin(str), std::end(str), std::begin(arg), true,
        [](const bool init, const bool val)
        { return init && val; },
        [](const auto lhschar, const auto rhschar)
        { return lhschar == rhschar; });
}

static_assert(check_equality(std::string_view("1234"), std::string_view("1234")));
static_assert(!check_equality(std::string_view("1234"), std::string_view("01234")));

enum struct ParseResult
{
    SUCCESS,
    SUCCESS_HELP_REQUESTED,
    WRONG_NO_ARGS,
    ARG_INVALID
};

struct Options_T
{
    pico_ws2812::WRGB new_value{};
    std::optional<size_t> pixel_idx{std::nullopt};
};

static std::pair<Options_T, ParseResult> parse_args(const auto &arg_array)
{
    /* Usage: SET w r g b
     *          OR
     *        SET w r g b idx
     */

    if (std::size(arg_array) != 1 && std::size(arg_array) != 4 && std::size(arg_array) != 5)
    {
        return std::make_pair(Options_T{}, ParseResult::WRONG_NO_ARGS);
    }

    if (check_equality(arg_array[0], "help"))
    {
        return std::make_pair(Options_T{}, ParseResult::SUCCESS_HELP_REQUESTED);
    }

    auto &&interpret_and_assign{[](const auto &argument_value, auto &result)
                                {
                                    const auto [_, ec]{std::from_chars(std::begin(argument_value), std::end(argument_value), result)};
                                    return ec == std::errc{};
                                }};

    uint8_t white, red, green, blue;
    if (!interpret_and_assign(arg_array[0], white))
    {
        return std::make_pair(Options_T{}, ParseResult::ARG_INVALID);
    }
    if (!interpret_and_assign(arg_array[1], red))
    {
        return std::make_pair(Options_T{}, ParseResult::ARG_INVALID);
    }
    if (!interpret_and_assign(arg_array[2], green))
    {
        return std::make_pair(Options_T{}, ParseResult::ARG_INVALID);
    }
    if (!interpret_and_assign(arg_array[3], blue))
    {
        return std::make_pair(Options_T{}, ParseResult::ARG_INVALID);
    }

    Options_T result{.new_value{.white{white}, .red{red}, .green{green}, .blue{blue}}};

    if (std::size(arg_array) == 5)
    {
        size_t idx;
        const auto char_to_int_succeeded{interpret_and_assign(arg_array[4], idx)};
        const auto pixel_idx_in_range{idx < std::size(pixel_buffer)};
        if (!char_to_int_succeeded || !pixel_idx_in_range)
        {
            return std::make_pair(Options_T{}, ParseResult::ARG_INVALID);
        }
        result.pixel_idx = idx;
    }

    return std::make_pair(result, ParseResult::SUCCESS);
}

static void print_usage()
{
    printf("Usage:\n");
    printf("  set WHITE RED GREEN BLUE\n");
    printf("  set WHITE RED GREEN BLUE INDEX\n");
    printf("  set help\n");
}
/* Implementation of the SET command for a pico board.
    Very not configurable right now
    PRECONDITIONS:
        stdio drivers are already setup, as it will use printf directly
        Specific PIO and state machines are available
        Specific Adafruit NeoPixel boards are in use, and connected to specific GPIOs
 */
Command_Result set_fn(const Command &args)
{
    const auto [opts, parse_status]{parse_args(args.arguments)};

    switch (parse_status)
    {
    case ParseResult::ARG_INVALID:
        print_usage();
        return Command_Result::ARG_INVALID;
    case ParseResult::WRONG_NO_ARGS:
        print_usage();
        return Command_Result::ARG_INVALID;
    case ParseResult::SUCCESS_HELP_REQUESTED:
        print_usage();
        return Command_Result::SUCCESS;
    case ParseResult::SUCCESS:
        break;
    }

    const auto &[value, pixidx]{opts};

    if (pixidx.has_value())
    {
        set_pixel_in_buffer(*pixidx, value);
    }
    else
    {
        fill_pixel_buffer(value);
    }

    put_pixel_buffer();

    return Command_Result::SUCCESS;
}
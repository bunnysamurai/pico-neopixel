#include "app/Command.hpp"

#include "pico/printf.h"

#include "ws2812/ws2812.hpp"

Command_Result set_fn([[maybe_unused]] const Command &args)
{
    printf("Not implemented yet.\n");
    return Command_Result::SUCCESS;
}
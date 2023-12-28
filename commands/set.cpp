#include "app/Command.hpp"

#include <iostream>

#include "ws2812/ws2812.hpp"

Command_Result set_fn([[maybe_unused]] const Command &args)
{
    std::cout << "Not implemented yet.\n";
    return Command_Result::SUCCESS;
}
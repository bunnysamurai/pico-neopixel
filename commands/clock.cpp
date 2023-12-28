#include "app/Command.hpp"

#include <iostream>

Command_Result clock_fn([[maybe_unused]] const Command &args)
{
    std::cout << "Not yet implemented\n";
    return Command_Result::SUCCESS;
}
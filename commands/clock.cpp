#include "app/Command.hpp"

#include "pico/printf.h"

Command_Result clock_fn([[maybe_unused]] const Command &args)
{
    printf("Not yet implemented\n");
    return Command_Result::SUCCESS;
}
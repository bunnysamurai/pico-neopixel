
#include "app/Command.hpp"

#include "pico/printf.h"

namespace
{
    void print_usage()
    {
        printf("Welcome to the Meven Light 5000.\n");
        printf("We'll keep the light on for ya.\n");
        printf("Available commands:\n");
        for (const auto &key : BASECMDS)
        {
            printf("  %s\n", std::data(key));
        }
        printf("\nIn general, type CMD help to see command specific help.\n");
    }
}

Command_Result help_fn(const Command &cmd)
{
    print_usage();
    return Command_Result::SUCCESS;
}

#include "app/Command.hpp"

#include <iostream>

namespace
{
    void print_usage()
    {
        std::cout << "Welcome to the Meven Light 5000.\n";
        std::cout << "We'll keep the light on for ya.\n";
        std::cout << "Available commands:\n";
        for (const auto &key : BASECMDS)
        {
            std::cout << "  " << key << '\n';
        }
        std::cout << "\nIn general, type CMD help to see command specific help.\n";
    }
}

Command_Result help_fn(const Command &cmd)
{
    print_usage();
    return Command_Result::SUCCESS;
}
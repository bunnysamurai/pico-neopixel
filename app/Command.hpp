#if !defined(COMMAND_HPP)
#define COMMAND_HPP

#include <array>
#include <cstddef>
#include <string_view>
#include <algorithm>
#include <numeric>
#include "pico/printf.h"
#include "variable_array.hpp"

enum struct Command_Result
{
    SUCCESS,
    COMMAND_NOT_FOUND
};

template <size_t ARG_LENGTH, size_t ARG_MAX_COUNT>
struct Command_T
{
public:
    using arg_type = std::array<char, ARG_LENGTH>;
    embp::variable_array<char, ARG_LENGTH> name;
    std::array<arg_type, ARG_MAX_COUNT> arguments;
};

using Command = Command_T<8, 5>;
using Command_Handler = Command_Result (*)(const Command &);

extern Command_Result help_fn(const Command &);
extern Command_Result set_fn(const Command &);
extern Command_Result pattern_fn(const Command &);
extern Command_Result clock_fn(const Command &);

inline constexpr std::array BASECMDS{
    std::string_view{"help"},
    std::string_view{"set"},
    std::string_view{"pattern"},
    std::string_view{"clock"}};
inline constexpr std::array CMDHANDLES{
    Command_Handler{help_fn},
    Command_Handler{set_fn},
    Command_Handler{pattern_fn},
    Command_Handler{clock_fn}};

constexpr Command_Handler lookup_fn(const auto &name, Command_Result &status)
{
    status = Command_Result::SUCCESS;
    const auto itr{std::find(std::begin(BASECMDS), std::end(BASECMDS), std::begin(name))};
    if (itr == std::end(BASECMDS))
    {
        status = Command_Result::COMMAND_NOT_FOUND;
        return {};
    }
    return *std::next(std::begin(CMDHANDLES), std::distance(std::begin(BASECMDS), itr));
}

[[nodiscard]] constexpr bool check_command_is_valid(const Command &cmd)
{
    auto start = std::begin(cmd.name);
    const auto finish = std::end(cmd.name);

    // we check each one character at a time across all possibilities
    // we keep track of which commands are not a match, and skip any additionaly processing if it has already failed
    // if this command possibility hasn't failed, it will fail if any are true:
    //  1. the length of the input pattern and the length of the command are not the same
    //  2. the charater in the input pattern does not match the letter in the command
    const size_t word_dis{static_cast<size_t>(std::distance(start, finish))};

    std::array<bool, std::size(BASECMDS)> failed{};

    // check word size
    for (size_t cmdidx{0}; cmdidx < std::size(BASECMDS); ++cmdidx)
    {
        const auto &command_possibility{BASECMDS[cmdidx]};
        if (word_dis != std::size(command_possibility))
        {
            failed[cmdidx] = true;
        }
    }

    // check string equality
    constexpr auto LONGEST_C0MMAND{[]
                                   {
                                       return std::transform_reduce(
                                           std::begin(BASECMDS), std::end(BASECMDS), size_t{0},
                                           [](auto init, auto val)
                                           { return std::max(init, val); },
                                           [](auto str)
                                           { return std::size(str); });
                                   }()};

    for (size_t charidx{0}; charidx < LONGEST_C0MMAND; ++charidx, ++start)
    {
        for (size_t cmdidx{0}; cmdidx < std::size(BASECMDS); ++cmdidx)
        {
            if (failed[cmdidx])
            {
                continue;
            }
            const auto &command_possibility{BASECMDS[cmdidx]};
            if (!(charidx < std::size(command_possibility)))
            {
                continue;
            }
            if (*start != command_possibility[charidx])
            {
                failed[cmdidx] = true;
                continue;
            }
        }
    }

    return !std::all_of(std::begin(failed), std::end(failed), [](auto val)
                        { return val; });
}

[[nodiscard]] constexpr Command_Result handle(const Command &cmd) noexcept
{
    Command_Result status{Command_Result::SUCCESS};
    auto &&fn{lookup_fn(cmd.name, status)};
    if (status != Command_Result::SUCCESS)
    {
        return status;
    }
    return fn(cmd);
}

template <class Console>
constexpr void print_error(Console &logger, Command_Result status) noexcept
{
    switch (status)
    {
    case Command_Result::SUCCESS:
        return;
    case Command_Result::COMMAND_NOT_FOUND:
        print_error(logger, "Command not found.");
        return;
    }
}

template <class CommandProvider, class Console>
class Command_SM
{
public:
    Command_SM(const char *console_str, CommandProvider &obj, Console &log) : m_commander{obj}, m_log{log}, m_console{console_str} {}

    constexpr void update() const
    {
        auto &&print_line{
            [&](const auto &line)
            {
                for (const auto c : line)
                {
                    print(m_log, "%c", c);
                }
                print(m_log, "\n");
            }};

        if (!command_available(m_commander))
        {
            return;
        }

        const Command next_cmd{get_next_command(m_commander)};

        print_line(next_cmd.name);

        if (!check_command_is_valid(next_cmd))
        {
            print_error(m_log, "Not a valid command.\n");
            print(m_log, m_console);
            return;
        }

        const auto status{handle(next_cmd)};
        if (status != Command_Result::SUCCESS)
        {
            print_error(m_log, status);
        }
        print(m_log, m_console);
    }

private:
    CommandProvider &m_commander;
    Console &m_log;
    const char *m_console;
};

#endif
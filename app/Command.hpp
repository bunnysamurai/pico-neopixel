#if !defined(COMMAND_HPP)
#define COMMAND_HPP

#include <array>
#include <cstddef>
#include <string_view>
#include <algorithm>

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
    const char *name;
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

constexpr Command_Handler lookup_fn(std::string_view name, Command_Result &status)
{
    status = Command_Result::SUCCESS;
    const auto itr{std::find(std::begin(BASECMDS), std::end(BASECMDS), name)};
    if (itr == std::end(BASECMDS))
    {
        status = Command_Result::COMMAND_NOT_FOUND;
        return {};
    }
    return *std::next(std::begin(CMDHANDLES), std::distance(std::begin(BASECMDS), itr));
}

template <class CommandProvider, class Console>
class Command_SM
{
public:
    Command_SM(CommandProvider obj, Console log) : m_commander{std::move(obj)}, m_log{std::move(log)} {}

    constexpr void update() const
    {
        if (!command_available(m_commander))
        {
            return;
        }
        const auto status{handle(get_next_command(m_commander))};
        if (!status)
        {
            print_error(m_log, status);
        }
    }

private:
    CommandProvider m_commander;
    Console m_log;
};

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

#endif
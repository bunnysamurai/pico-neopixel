#if !defined(INPUT_STATE_MACHINE_HPP)
#define INPUT_STATE_MACHINE_HPP

// proces_input gets called constantly in the super-loop
// it will receive characters, and as soon as it receives a word (i.e. encounters a space character), it will add an event to a ring buffer
// the ring buffer will be consumed by the Input State Machine as events become available
// the input state machine will attach command specific state machines to handle the requested command

#include <numeric>
#include <algorithm>
#include <utility>

#include "pico/stdio.h"

#include "Ring_Buffer.hpp"
#include "Command.hpp"
#include "variable_array.hpp"
#include "pico_panic.hpp"

template <class LineProvider, class Logger>
class CommandBuilder_SM
{
public:
    constexpr CommandBuilder_SM(LineProvider &obj, Logger &log) noexcept : m_read_line_fn{obj}, m_logger{log} {}

    constexpr void update() noexcept
    {
        if (!available(m_read_line_fn))
        {
            return;
        }
        const auto new_line{get_next_line(m_read_line_fn)};
        auto search_itr{std::begin(new_line)};

        const auto [word_start, word_finish]{parse_word(new_line, search_itr)};
        add_name_to_command(word_start, word_finish);

        while (search_itr != std::end(new_line))
        {
            const auto [word_start, word_finish]{parse_word(new_line, search_itr)};
            add_arg_to_command(word_start, word_finish);
        }

        enqueue_command();
    }

    [[nodiscard]] constexpr bool command_available() const noexcept
    {
        return !m_cmd_buffer.empty();
    }

    [[nodiscard]] constexpr Command get_next_command() noexcept
    {
        const auto tmp{m_cmd_buffer.dequeue()};
        return tmp;
    }

private:
    LineProvider &m_read_line_fn;
    Fixed_Log2_Ring_Buffer<Command, 4> m_cmd_buffer;
    Command m_tmp;
    Logger &m_logger;

    [[nodiscard]] static constexpr auto parse_word(const auto &line, auto &search_itr) noexcept
    {
        constexpr char SPACE{' '};
        const auto search_begin{search_itr};
        search_itr = std::find(search_itr, std::end(line), SPACE);
        const auto search_end{search_itr};
        while (search_itr != std::end(line) && *search_itr == SPACE)
        {
            ++search_itr;
        }
        return std::make_pair(search_begin, search_end);
    }

    constexpr void enqueue_command() noexcept
    {
        if (!m_cmd_buffer.enqueue(m_tmp))
        {
            print_error(m_logger, "PANIC Unable to enqueue the command!");
            pico::panic::loop_forever();
        }
        m_tmp.arguments.clear();
        m_tmp.name.clear();
    }

    constexpr void add_name_to_command(const auto word_start, const auto word_finish) noexcept
    {
        const auto ssize{std::distance(word_start, word_finish)};
        m_tmp.name.resize(ssize);
        memcpy(std::data(m_tmp.name), word_start, ssize);
    }

    constexpr void add_arg_to_command(const auto word_start, const auto word_finish) noexcept
    {
        const auto ssize{std::distance(word_start, word_finish)};
        std::decay_t<decltype(m_tmp.arguments[0])> tmp;
        tmp.resize(ssize);
        memcpy(std::data(tmp), word_start, ssize);
        m_tmp.arguments.push_back(tmp);
    }
};

template <class LineProvider, class Logger>
bool command_available(CommandBuilder_SM<LineProvider, Logger> &sm)
{
    return sm.command_available();
}

template <class LineProvider, class Logger>
Command get_next_command(CommandBuilder_SM<LineProvider, Logger> &sm)
{
    return sm.get_next_command();
}

// an example line LineProvider
// * a line reading state machine is pretty simple, if you read a /n then stuff the current buffer
template <size_t MAX_LINE_LENGTH>
class PicoLineProvider
{
public:
    using line_type = embp::variable_array<char, MAX_LINE_LENGTH>;

    void update() noexcept
    {
        int c{getchar_timeout_us(0)};
        if (c == PICO_ERROR_TIMEOUT)
        {
            return;
        }
        printf("%c", c); // TODO we also echo here?  need to support backspace and delete
        if (c == '\n')
        {
            if (!m_line_buffer.enqueue(m_current_line))
            {
                printf("PANIC hmm, couldn't enqueue?");
                pico::panic::loop_forever();
            }
            m_current_line.clear();
            return;
        }
        m_current_line.push_back(c);
    }

    [[nodiscard]] bool line_available() const noexcept
    {
        return !m_line_buffer.empty();
    }
    [[nodiscard]] line_type get_next_line() noexcept
    {
        const auto tmp{m_line_buffer.dequeue()};
        return tmp;
    }

private:
    static constexpr size_t LINE_BUFFER_CAPACITY{4};
    Fixed_Log2_Ring_Buffer<line_type, LINE_BUFFER_CAPACITY> m_line_buffer;
    line_type m_current_line;
};

template <size_t N>
auto available(PicoLineProvider<N> &line_jobby)
{
    return line_jobby.line_available();
}
template <size_t N>
auto get_next_line(PicoLineProvider<N> &line_jobby)
{
    return line_jobby.get_next_line();
}
#endif
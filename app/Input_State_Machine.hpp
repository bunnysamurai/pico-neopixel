#if !defined(INPUT_STATE_MACHINE_HPP)
#define INPUT_STATE_MACHINE_HPP

// proces_input gets called constantly in the super-loop
// it will receive characters, and as soon as it receives a word (i.e. encounters a space character), it will add an event to a ring buffer
// the ring buffer will be consumed by the Input State Machine as events become available
// the input state machine will attach command specific state machines to handle the requested command

#include <numeric>
#include <algorithm>

#include "pico/stdio.h"

#include "Ring_Buffer.hpp"
#include "Command.hpp"
#include "variable_array.hpp"
#include "pico_panic.hpp"

template <class LineProvider, class Logger>
class SM
{
public:
    enum struct State
    {

    };

    SM(LineProvider &obj, Logger &log) : m_read_line_fn{obj}, m_logger{log} {}

    void update()
    {
        if (!available(m_read_line_fn))
        {
            return;
        }
        const auto new_line{get_next_line(m_read_line_fn)};
        const auto word{parse_word(new_line)};

        // TODO was tired when writing this.  check it's okay
        m_tmp.name.resize(std::size(word));
        memcpy(&m_tmp.name, std::data(word), std::size(word));

        if (!m_cmd_buffer.enqueue(m_tmp))
        {
            print_error(m_logger, "PANIC Unable to enqueue the command!");
            pico::panic::loop_forever();
        }
    }

    bool command_available() const noexcept
    {
        return !m_cmd_buffer.empty();
    }

    Command get_next_command() noexcept
    {
        return m_cmd_buffer.dequeue();
    }

private:
    LineProvider &m_read_line_fn;
    Fixed_Log2_Ring_Buffer<Command, 4> m_cmd_buffer;
    Command m_tmp;
    std::array<char, 128> char_buffer;
    Logger &m_logger;

    template <class T>
    T parse_word(const T &arg)
    {
        return arg;
    }
};

template <class LineProvider, class Logger>
bool command_available(SM<LineProvider, Logger> &sm)
{
    return sm.command_available();
}

template <class LineProvider, class Logger>
Command get_next_command(SM<LineProvider, Logger> &sm)
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
        return m_line_buffer.dequeue();
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
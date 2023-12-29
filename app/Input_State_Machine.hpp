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

template <class LineProvider, class Logger>
class SM
{
public:
    enum struct State
    {

    };

    SM(LineProvider obj, Logger log) : m_read_line_fn{std::move(obj)}, m_logger{std::move(log)} {}

    void update()
    {
        if (!available(m_read_line_fn))
        {
            return;
        }
        const auto new_line{get_next_line(m_read_line_fn)};
        const auto word{parse_word(new_line)};

        if (!intialize_command(std::begin(word), std::end(word)))
        {
            print_error(m_logger, "word is not a valid command");
            return;
        }
    }

private:
    LineProvider m_read_line_fn;
    Fixed_Log2_Ring_Buffer<Command, 4> m_cmd_buffer;
    Command m_tmp;
    std::array<char, 128> char_buffer;
    Logger m_logger;

    template <class Iter>
    bool intialize_command(Iter start, Iter finish)
    {
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
        static constexpr auto LONGEST_C0MMAND{[]
                                              {
                                                  return std::transform_reduce(std::begin(BASECMDS), std::end(BASECMDS), std::max, [](auto str)
                                                                               { return std::size(str); });
                                              }()};

        for (size_t charidx{0}; charidx < LONGEST_C0MMAND; ++charidx)
        {
            for (size_t cmdidx{0}; cmdidx < std::size(BASECMDS); ++cmdidx)
            {
                if (failed[cmdidx])
                {
                    continue;
                }
                const auto &command_possibility{BASECMDS[cmdidx]};
                if (*start != command_possibility[charidx])
                {
                    failed[cmdidx] = true;
                    continue;
                }
            }
        }

        // if there are any successes, and there should only be one, grab it and set m_tmp
        if (std::all_of(std::begin(failed), std::end(failed), [](auto val)
                        { return val; }))
        {
            return false;
        }
        const auto idx_found{std::distance(std::begin(failed), std::find(std::begin(failed), std::end(failed), true))};
        m_tmp.name = std::data(BASECMDS[idx_found]);
        return true;
    }
};

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
                printf("hmm, couldn't enqueue?");
            }
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
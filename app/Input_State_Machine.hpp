#if !defined(INPUT_STATE_MACHINE_HPP)
#define INPUT_STATE_MACHINE_HPP

// proces_input gets called constantly in the super-loop
// it will receive characters, and as soon as it receives a word (i.e. encounters a space character), it will add an event to a ring buffer
// the ring buffer will be consumed by the Input State Machine as events become available
// the input state machine will attach command specific state machines to handle the requested command


#include "Ring_Buffer.hpp"
#include "Command.hpp"

template <class LineProvider>
class SM
{
public:
    enum struct State
    {

    };

    SM(LineProvider obj) : m_read_line_fn{std::move(obj)};

    void update()
    {
        if (available(m_read_line_fn))
        {
            next_line = read_line(m_read_line_fn);
        }
    }

private:
    LineProvider m_read_line_fn;
    Fixed_Log2_Ring_Buffer<Command, 4> m_cmd_buffer;

};

#endif
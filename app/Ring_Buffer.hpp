#if !defined(RING_BUFFER_HPP)
#define RING_BUFFER_HPP

#include <array>
#include <bit>
#include <cstddef>

template <class T, size_t MAX_SIZE>
    requires(std::popcount(MAX_SIZE) == 1)
class Fixed_Log2_Ring_Buffer
{
public:
    [[nodiscard]] constexpr bool full() const noexcept
    {
        return next(m_put) == m_get;
    }
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return m_put == m_get;
    }
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        if (m_put >= m_get)
        {
            return m_put - m_get;
        }
        return (MAX_SIZE - m_get) + m_put;
    }
    [[nodiscard]] constexpr size_t capacity() const noexcept
    {
        return MAX_SIZE;
    }

    [[nodiscard]] constexpr bool enqueue(T value) noexcept
    {
        if (full())
        {
            return false;
        }
        m_buf[m_put] = std::move(value);
        inc(m_put);
        return true;
    }
    /**
     * @brief pop the oldest element inserted. Behaviour undefined if buffer is empty.
     */
    [[nodiscard]] constexpr T dequeue() noexcept
    {
        const auto tmp{m_buf[m_get]};
        inc(m_get);
        return tmp;
    }

private:
    static constexpr size_t INDEX_MASK{MAX_SIZE - 1};
    std::array<T, MAX_SIZE> m_buf;
    size_t m_put{0};
    size_t m_get{0};

    static constexpr size_t next(size_t counter) noexcept
    {
        return (counter + 1) & INDEX_MASK;
    }
    static constexpr void inc(size_t &counter) noexcept
    {
        counter = next(counter);
    }
};

namespace tests
{
    [[nodiscard]] constexpr bool run_ring_buffer_tests()
    {
        bool rv{true};

        Fixed_Log2_Ring_Buffer<int, 8> dut;

        // =========================================
        rv &= dut.capacity() == 8;
        rv &= dut.empty();
        rv &= dut.size() == 0;

        rv &= dut.enqueue(42);
        rv &= dut.size() == 1;

        rv &= dut.enqueue(43);
        rv &= dut.size() == 2;

        rv &= dut.enqueue(44);
        rv &= dut.size() == 3;

        rv &= dut.enqueue(45);
        rv &= dut.size() == 4;

        rv &= dut.enqueue(46);
        rv &= dut.size() == 5;

        rv &= dut.enqueue(47);
        rv &= dut.size() == 6;

        rv &= dut.enqueue(48);
        rv &= dut.size() == 7;

        rv &= dut.full();

        rv &= dut.enqueue(49) == false;
        rv &= dut.size() == 7;

        rv &= dut.dequeue() == 42;
        rv &= dut.dequeue() == 43;
        rv &= dut.dequeue() == 44;
        rv &= dut.dequeue() == 45;
        rv &= dut.dequeue() == 46;
        rv &= dut.dequeue() == 47;
        rv &= dut.dequeue() == 48;
        rv &= dut.empty();

        // =========================================
        rv &= dut.capacity() == 8;
        rv &= dut.empty();
        rv &= dut.size() == 0;

        rv &= dut.enqueue(42);
        rv &= dut.size() == 1;

        rv &= dut.enqueue(43);
        rv &= dut.size() == 2;

        rv &= dut.enqueue(44);
        rv &= dut.size() == 3;

        rv &= dut.enqueue(45);
        rv &= dut.size() == 4;

        rv &= dut.enqueue(46);
        rv &= dut.size() == 5;

        rv &= dut.enqueue(47);
        rv &= dut.size() == 6;

        rv &= dut.enqueue(48);
        rv &= dut.size() == 7;

        rv &= dut.full();

        rv &= dut.enqueue(49) == false;
        rv &= dut.size() == 7;

        rv &= dut.dequeue() == 42;
        rv &= dut.dequeue() == 43;
        rv &= dut.dequeue() == 44;
        rv &= dut.dequeue() == 45;
        rv &= dut.dequeue() == 46;
        rv &= dut.dequeue() == 47;
        rv &= dut.dequeue() == 48;
        rv &= dut.empty();

        return rv;
    }
    static_assert(run_ring_buffer_tests());
}
#endif
#pragma once
#ifdef STD_LIB_AVAILABLE
#include <cstddef>
#ifndef NDEBUG
#include <iostream>
#endif
#else
#include <stddef.h>
#endif
#include "utility.hpp"
#include "reverse_iterator.hpp"

// like C++17 vector, ish
// think about builtin, trival, and non-trivial types
/* data_ must be initialized in constexpr constructors.
 * Does this add unnecessary work in non-constexpr contexts?
 */
// More inline?

namespace embp
{

template < class DataType, size_t Capacity >
struct variable_array
{
public:
    // type defs required
    using value_type = DataType;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = embp::reverse_iterator<iterator>;
    using const_reverse_iterator = embp::reverse_iterator<const_iterator>;

private:
    value_type data_[Capacity];
    size_type size_{0};

    template < class IterOrValue, class ResolverFunc >
    constexpr void size_copy_impl(IterOrValue &&first, const size_type start, const size_type count, ResolverFunc resolve) noexcept
    {
        //TODO: no side effects in constexpr contexts
        for(size_type ii = start; ii < start + count; ++ii)
            data_[ii] = resolve(embp::forward<IterOrValue>(first));
    }

    constexpr bool is_available(const size_type request) noexcept
    {
        return request < Capacity + 1;
    }

    constexpr void shift_subarray_impl(const_iterator pos, const difference_type dis) noexcept
    {
        auto it = dis > 0 ? this->end() - 1 : this->begin() + embp::distance(this->cbegin(), pos);
        auto end_itr = dis > 0 ? pos - 1 : this->cend();
        while(it != end_itr)
        {
            *(it+dis) = *it;
            it += 1 - 2*(dis>0);
        }
    }

public:
    // Constructors
    constexpr variable_array() = default;
    constexpr explicit variable_array(size_type n) noexcept : size_{ n * is_available(n) }, data_{} { }
    constexpr explicit variable_array(size_type n, const value_type &value) noexcept :
        data_{}
    {
        assign(n, value);
    }
    template < class Iter >
    constexpr variable_array(Iter first, Iter last) noexcept :
        data_{}
    {
        assign(first, last);
    }
    constexpr variable_array(const variable_array &cpy) noexcept :
        data_{}
    {
        assign(cpy.begin(), cpy.end());
    }
    constexpr variable_array(variable_array &&mv) noexcept :
        size_{mv.size()}, data_{}
    {
        size_copy_impl(mv, 0, size_, [ii=size_type{}] (auto &&rv) mutable -> value_type&& { return embp::move(rv[ii++]); });
    }
    //variable_array(std::initializer_list)

    // Assignment
    constexpr variable_array& operator=(const variable_array &cpy) noexcept
    {
        assign(cpy.begin(), cpy.end());
        return *this;
    }
    constexpr variable_array& operator=(variable_array &&mv) noexcept
    {
        size_ = mv.size();
        size_copy_impl(mv, 0, size_, [ii=size_type{}] (auto &&rv) mutable -> value_type&& { return embp::move(rv[ii++]); });
        return *this;
    }
    constexpr void assign(size_type n, const value_type &value) noexcept
    {
        size_ = n * is_available(n);
        size_copy_impl(value, 0, size_, [](const auto &rv) -> const value_type& { return rv; });
    }
    template < class Iter >
    constexpr void assign(Iter first, Iter last) noexcept
    {
        size_ = (last - first) * is_available(last-first);
        size_copy_impl(first, 0, size_, [](auto &rv) { return *rv++; });
    }
    //assign(std::initializer_list)
    //get_allocator

    // Element access
    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept { return data_[pos]; }
    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept { return data_[pos]; }
    [[nodiscard]] constexpr reference front() noexcept { return data_[0]; }
    [[nodiscard]] constexpr const_reference front() const noexcept { return data_[0]; }
    [[nodiscard]] constexpr reference back() noexcept { return data_[size_ - 1]; }
    [[nodiscard]] constexpr const_reference back() const noexcept { return data_[size_ - 1]; }
    [[nodiscard]] constexpr pointer data() { return data_; }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return data_; }

    // Iterators
    [[nodiscard]] constexpr iterator begin() noexcept { return &data_[0]; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return &data_[0]; }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return &data_[0]; }
    [[nodiscard]] constexpr iterator end() noexcept { return &data_[0] + size_; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return &data_[0] + size_; }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return &data_[0] + size_; }
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(this->end()); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(this->end()); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(this->cend()); }
    [[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator(this->begin()); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(this->begin()); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(this->cbegin()); }

    // Capacity
    [[nodiscard]] constexpr bool empty() const noexcept { return this->begin() == this->end(); }
    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
    [[nodiscard]] constexpr size_type max_size() const noexcept { return Capacity; }
    //reserve
    [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }
    //shrink_to_fit

    // Modifiers
    constexpr void clear() noexcept { size_ = 0; }
    constexpr iterator insert(const_iterator pos, const value_type& value) noexcept
    {
        if(is_available(size_+1))
        {
            shift_subarray_impl(pos, 1);
            ++size_;
            *(this->begin() + embp::distance(this->cbegin(), pos)) = value;
            return this->begin() + embp::distance(this->cbegin(), pos);
        }
        else
        {
            return this->end();
        }
    }
    constexpr iterator insert(const_iterator pos, value_type&& value) noexcept
    {
        return insert(pos, value);
    }
    constexpr iterator insert(const_iterator pos, size_type count, const value_type& value) noexcept
    {
        if(is_available(size_+count))
        {
            shift_subarray_impl(pos, count);
            size_ += count;
            auto it = this->begin() + embp::distance(this->cbegin(), pos);
            const auto end_it = it + count;
            for(; it != end_it; ++it)
            {
                *it = value;
            }
            return this->begin() + embp::distance(this->cbegin(), pos);
        }
        else
        {
            return this->end();
        }
    }
    template < class InputIterator >
    constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) noexcept
    {
        const auto count = embp::distance(first, last);
        if(is_available(size_+count))
        {
            size_ += count;
            shift_subarray_impl(pos, count);
            auto it = this->begin() + embp::distance(this->cbegin(), pos);
            const auto end_it = it + count;
            for(; it != end_it; ++it, ++first)
            {
                *it = *first;
            }
            return this->begin() + embp::distance(this->cbegin(), pos);
        }
        else
        {
            return this->end();
        }
    }
    //insert(const_iterator pos, std::initializer_list)
//    template < class ... Args >
//    constexpr iterator emplace(Args&&... args) noexcept;
    constexpr iterator erase(const_iterator pos) noexcept
    {
        return this->erase(pos, pos+1);
    }
    constexpr iterator erase(const_iterator first, const_iterator last) noexcept
    {
        const auto dis = embp::distance(first, last);
        const auto count = dis * (dis > 0);
        shift_subarray_impl(last, -count);
        size_ -= count;
        return this->begin() + embp::distance(this->cbegin(), first);
    }
    constexpr void push_back(const value_type &value) noexcept
    {
        if(size_ < Capacity)
        {
            ++size_;
            this->back() = value;
        }
    }
    constexpr void push_back(value_type &&value) noexcept
    {
        if(size_ < Capacity)
        {
            ++size_;
            this->back() = static_cast<value_type&&>(value);
        }
    }
//    template < class ... Args >
//    constexpr reference emplace_back(Args&&... args) noexcept;
    constexpr void pop_back() noexcept { size_ -= 1; }
    constexpr void resize(const size_type count) noexcept
    {
        this->resize(count, value_type{});
    }
    constexpr void resize(const size_type count, const value_type &value) noexcept
    {
        const auto prev_size = size_;
        size_ = count * is_available(count);
        if(prev_size < size_)
        {
            size_copy_impl(value, prev_size, size_ - prev_size, [](const auto &rv) -> const value_type& { return rv; });
        }
    }
    constexpr void swap(variable_array &other) noexcept
    {
        auto& smallerArray = this->size() > other.size() ? other : *this;
        auto& largerArray = this->size() > other.size() ? *this : other;

        const auto smallSizeCount = smallerArray.size();
        for(size_type ii = 0; ii != smallSizeCount; ++ii)
        {
            embp::swap(smallerArray[ii], largerArray[ii]);
        }
        smallerArray.resize(largerArray.size());
        for(size_type ii = smallSizeCount; ii != smallerArray.size(); ++ii)
        {
            smallerArray[ii] = largerArray[ii];
        }
        largerArray.resize(smallSizeCount);
    }

};

// Non-member functions
template < class DataType, size_t Count >
bool operator==(const variable_array<DataType, Count> &lhs, const variable_array<DataType, Count> &rhs) noexcept
{
    return
            lhs.size() == rhs.size()
            &&
            embp::compareElementWise(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto &l, const auto &r){ return l == r; });
}
template < class DataType, size_t Count >
bool operator!=(const variable_array<DataType, Count> &lhs, const variable_array<DataType, Count> &rhs) noexcept
{
    return !(lhs == rhs);
}
template < class DataType, size_t Count >
bool operator<(const variable_array<DataType, Count> &lhs, const variable_array<DataType, Count> &rhs) noexcept
{
    return embp::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto &l, const auto &r){ return l < r; });

}
template < class DataType, size_t Count >
bool operator<=(const variable_array<DataType, Count> &lhs, const variable_array<DataType, Count> &rhs) noexcept
{
    return embp::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), lhs.cend(), [](const auto &l, const auto &r){ return l <= r; });
}
template < class DataType, size_t Count >
bool operator>(const variable_array<DataType, Count> &lhs, const variable_array<DataType, Count> &rhs) noexcept
{
    return !(lhs <= rhs);
}
template < class DataType, size_t Count >
bool operator>=(const variable_array<DataType, Count> &lhs, const variable_array<DataType, Count> &rhs) noexcept
{
    return !(lhs < rhs);
}

template < class DataType, size_t Count >
void swap(variable_array<DataType, Count> &lhs, variable_array<DataType, Count> &rhs) noexcept
{
    lhs.swap(rhs);
}

} // embp

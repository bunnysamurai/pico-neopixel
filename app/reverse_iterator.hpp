#pragma once
#include "utility.hpp"

namespace embp
{
    template < class Iter >
    struct reverse_iterator
    {
    public:
        using iterator_type = Iter;
        using value_type = typename embp::iterator_traits<Iter>::value_type;
        using difference_type = typename embp::iterator_traits<Iter>::difference_type;
        using pointer = typename embp::iterator_traits<Iter>::pointer;
        using reference = typename embp::iterator_traits<Iter>::reference;
#ifdef STD_LIB_AVAILABLE
        using iterator_category = typename embp::iterator_traits<Iter>::iterator_category; // TODO: need to check for stdlib availability here
#endif
    private:
        iterator_type it_;
    public:
        constexpr reverse_iterator() = default;
        explicit constexpr reverse_iterator( Iter it ) : it_{it} { }
        template < class T >
        explicit constexpr reverse_iterator( const reverse_iterator<T> &other ) : reverse_iterator(other.base()) { }

        template < class T >
        constexpr reverse_iterator& operator=( const reverse_iterator<T> &other);

        constexpr iterator_type base() const 
        { 
            return it_;
        }

        constexpr reference operator*() const { auto tmp{it_}; return *--tmp; }
        constexpr pointer operator->() const { return &(operator*()); /* problem, what happens of the object has overloaded the addressof operator? */ };
        constexpr reference operator[](difference_type n)
        {
            return base()[-n-1];
        }

        constexpr reverse_iterator& operator++()
        {
            --it_;
            return *this;
        }
        constexpr reverse_iterator& operator++(int)
        {
            auto tmp{*this};
            (void)operator++();
            return tmp;
        }
        constexpr reverse_iterator& operator+=(const difference_type n)
        {
            //hmm, depending on the type of base iterator, this may not be correct
            it_ -= n;
            return *this;
        }
        constexpr reverse_iterator operator+(const difference_type n) const
        {
            auto tmp{*this};
            return tmp += n;
        }
        constexpr reverse_iterator& operator--()
        {
            ++it_;
            return *this;
        }
        constexpr reverse_iterator& operator--(int)
        {
            auto tmp{*this};
            (void)operator--();
            return tmp;
        }
        constexpr reverse_iterator& operator-=(const difference_type n)
        {
            //hmm, depending on the type of base iterator, this may not be correct
            it_ += n;
            return *this;
        }
        constexpr reverse_iterator& operator-(const difference_type n) const
        {
            auto tmp{*this};
            return tmp -= n;
        }
        
    };

    template < class Iter1, class Iter2 >
    [[nodiscard]] constexpr bool operator==(const reverse_iterator<Iter1> lhs, const reverse_iterator<Iter2> rhs) noexcept
    {
        return lhs.base() == rhs.base();
    }
    template < class Iter1, class Iter2 >
    [[nodiscard]] constexpr bool operator!=(const reverse_iterator<Iter1> lhs, const reverse_iterator<Iter2> rhs) noexcept
    {
        return !(lhs == rhs);
    }
    template < class Iter1, class Iter2 >
    [[nodiscard]] constexpr bool operator<(const reverse_iterator<Iter1> lhs, const reverse_iterator<Iter2> rhs) noexcept
    {
        return lhs.base() > rhs.base();
    }
    template < class Iter1, class Iter2 >
    [[nodiscard]] constexpr bool operator>(const reverse_iterator<Iter1> lhs, const reverse_iterator<Iter2> rhs) noexcept
    {
        return lhs.base() < rhs.base();
    }
    template < class Iter1, class Iter2 >
    [[nodiscard]] constexpr bool operator<=(const reverse_iterator<Iter1> lhs, const reverse_iterator<Iter2> rhs) noexcept
    {
        return !(lhs > rhs);
    }
    template < class Iter1, class Iter2 >
    [[nodiscard]] constexpr bool operator>=(const reverse_iterator<Iter1> lhs, const reverse_iterator<Iter2> rhs) noexcept
    {
        return !(lhs < rhs);
    }


} // embp

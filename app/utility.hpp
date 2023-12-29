#pragma once
#ifdef STD_LIB_AVAILABLE
#include <cstddef>
#else
#include <stddef.h>
#endif

namespace embp
{
    // Move, forward implementation

    /// integral_constant
    template <typename Type, Type val>
    struct integral_constant
    {
        static constexpr Type value = val;
        using value_type = Type;
        using type = integral_constant<Type, val>;
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };

    template <typename Type, Type val>
    constexpr Type integral_constant<Type, val>::value;
    /// The type used as a compile-time boolean with true value.
    using true_type = integral_constant<bool, true>;
    /// The type used as a compile-time boolean with false value.
    using false_type = integral_constant<bool, false>;

    /// is_lvalue_reference
    template <typename>
    struct is_lvalue_reference : public false_type
    {
    };

    template <typename T>
    struct is_lvalue_reference<T &> : public true_type
    {
    };

    template <typename Type>
    struct remove_reference
    {
        using type = Type;
    };

    template <typename Type>
    struct remove_reference<Type &>
    {
        using type = Type;
    };

    template <typename Type>
    struct remove_reference<Type &&>
    {
        using type = Type;
    };

    template <class Type>
    constexpr Type &&forward(typename remove_reference<Type>::type &value) noexcept
    {
        return static_cast<Type &&>(value);
    }

    template <class Type>
    constexpr Type &&forward(typename remove_reference<Type>::type &&value) noexcept
    {
        // need to static assert that the value is indeed an rvalue reference, or rather, not an lvalue reference
        static_assert(!is_lvalue_reference<Type>::value, "cannot forward an lvalue reference");
        return static_cast<Type &&>(value);
    }

    template <class Type>
    constexpr typename remove_reference<Type>::type &&move(Type &&t) noexcept
    {
        return static_cast<typename remove_reference<Type>::type &&>(t);
    }

    // Remove pointer
    template <class T>
    struct remove_pointer
    {
        using type = T;
    };

    template <class T>
    struct remove_pointer<T *>
    {
        using type = T;
    };

    template <class T>
    struct remove_pointer<T *const>
    {
        using type = T;
    };

    template <class T>
    struct remove_pointer<T *volatile>
    {
        using type = T;
    };

    template <class T>
    struct remove_pointer<T *const volatile>
    {
        using type = T;
    };

    template <class T>
    using remove_pointer_t = typename remove_pointer<T>::type;

    // Add pointer
    template <class T>
    struct type_identity
    {
        using type = T;
    };
    template <class T>
    auto try_add_pointer(int) -> type_identity<typename remove_reference<T>::type *>;
    template <class T>
    auto try_add_pointer(...) -> type_identity<T>;

    template <class T>
    struct add_pointer : decltype(try_add_pointer<T>(0))
    {
    };

    template <class T>
    using add_pointer_t = typename add_pointer<T>::type;

    // Iterator traits (thanks, pointer!)
    template <class Iter>
    struct iterator_traits
    {
        using value_type = typename Iter::value_type;
        using difference_type = typename Iter::difference_type;
        using pointer = typename Iter::pointer;
        using reference = typename Iter::reference;
#ifdef STD_LIB_AVAILABLE
        using iterator_category = typename Iter::iterator_category;
#endif
    };
    template <class T>
    struct iterator_traits<T *>
    {
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T *;
        using reference = T &;
#ifdef STD_LIB_AVAILABLE
        using iterator_category = std::random_access_iterator_tag;
#endif
    };
    template <class T>
    struct iterator_traits<const T *>
    {
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;
#ifdef STD_LIB_AVAILABLE
        using iterator_category = std::random_access_iterator_tag;
#endif
    };

    // Other utilities
    template <class InputIterator>
    constexpr ptrdiff_t distance(InputIterator first, InputIterator last) noexcept
    {
        return last - first;
    }
    template <class InputIterator, class InputIterator2>
    constexpr ptrdiff_t distance(InputIterator first, InputIterator2 last) noexcept
    {
        return last - first;
    }

    template <class T>
    inline constexpr void swap(T &lhs, T &rhs)
    {
        auto tmp{lhs};
        lhs = rhs;
        rhs = tmp;
    }
    template <class Type, size_t N>
    inline constexpr void swap(Type (&lhs)[N], Type (&rhs)[N])
    {
        for (size_t ii = 0; ii < N; ++ii)
        {
            embp::swap(lhs[ii], rhs[ii]);
        }
    }

    template <class InputIterator, class CompareOperator>
    constexpr bool compareElementWise(InputIterator lhs_first, InputIterator lhs_last, InputIterator rhs_first, InputIterator rhs_last, CompareOperator compare) noexcept
    {
        if (distance(lhs_first, lhs_last) != distance(rhs_first, rhs_last))
        {
            return false;
        }
        while (lhs_first != lhs_last)
        {
            if (!compare(*lhs_first++, *rhs_first++))
            {
                return false;
            }
        }
        return true;
    }

    template <class InputIt1, class InputIt2, class Compare>
    bool lexicographical_compare(InputIt1 first1, InputIt1 last1,
                                 InputIt2 first2, InputIt2 last2,
                                 Compare comp)
    {
        for (; (first1 != last1) && (first2 != last2); ++first1, (void)++first2)
        {
            if (comp(*first1, *first2))
                return true;
            if (comp(*first2, *first1))
                return false;
        }
        return (first1 == last1) && (first2 != last2);
    }

} // embp

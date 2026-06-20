#pragma once

#include <cstddef>

namespace mystl
{
    // ========================================================================
    // ITERATOR TAGS
    // ========================================================================

    struct input_iterator_tag {};
    struct output_iterator_tag {};

    struct forward_iterator_tag : input_iterator_tag {};
    struct bidirectional_iterator_tag : forward_iterator_tag {};
    struct random_access_iterator_tag : bidirectional_iterator_tag {};

    // ========================================================================
    // ITERATOR TRAITS
    // ========================================================================

    template <typename Iterator>
    struct iterator_traits
    {
        using difference_type   = typename Iterator::difference_type;
        using value_type        = typename Iterator::value_type;
        using pointer           = typename Iterator::pointer;
        using reference         = typename Iterator::reference;
        using iterator_category = typename Iterator::iterator_category;
    };

    template <typename T>
    struct iterator_traits<T*>
    {
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        using iterator_category = random_access_iterator_tag;
    };

    template <typename T>
    struct iterator_traits<const T*>
    {
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = const T*;
        using reference         = const T&;
        using iterator_category = random_access_iterator_tag;
    };

    // ========================================================================
    // DISTANCE
    // ========================================================================

    template <typename InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last)
    {
        typename iterator_traits<InputIterator>::difference_type n = 0;

        while (first != last)
        {
            ++first;
            ++n;
        }

        return n;
    }

    // ========================================================================
    // ADVANCE
    // ========================================================================

    template <typename InputIterator, typename Distance>
    void advance(InputIterator& it, Distance n)
    {
        while (n > 0)
        {
            ++it;
            --n;
        }
    }

    // ========================================================================
    // NEXT
    // ========================================================================

    template <typename Iterator>
    Iterator next(Iterator it)
    {
        ++it;
        return it;
    }

    template <typename Iterator>
    Iterator next(Iterator it,
                typename iterator_traits<Iterator>::difference_type n)
    {
        advance(it, n);
        return it;
    }

    // ========================================================================
    // PREV
    // ========================================================================

    template <typename BidirectionalIterator>
    BidirectionalIterator prev(BidirectionalIterator it)
    {
        --it;
        return it;
    }

    template <typename BidirectionalIterator>
    BidirectionalIterator prev(
        BidirectionalIterator it,
        typename iterator_traits<BidirectionalIterator>::difference_type n)
    {
        while (n > 0)
        {
            --it;
            --n;
        }

        return it;
    }

    // ========================================================================
    // REVERSE ITERATOR
    // ========================================================================

    template <typename Iterator>
    class reverse_iterator
    {
    public:
        using iterator_type     = Iterator;
        using traits_type       = iterator_traits<Iterator>;

        using iterator_category = typename traits_type::iterator_category;
        using value_type        = typename traits_type::value_type;
        using difference_type   = typename traits_type::difference_type;
        using pointer           = typename traits_type::pointer;
        using reference         = typename traits_type::reference;

    private:
        Iterator current_;

    public:
        constexpr reverse_iterator()
            : current_()
        {
        }

        explicit constexpr reverse_iterator(Iterator it)
            : current_(it)
        {
        }

        constexpr Iterator base() const
        {
            return current_;
        }

        constexpr reference operator*() const
        {
            Iterator tmp = current_;
            --tmp;
            return *tmp;
        }

        constexpr pointer operator->() const
        {
            return &(operator*());
        }

        constexpr reverse_iterator& operator++()
        {
            --current_;
            return *this;
        }

        constexpr reverse_iterator operator++(int)
        {
            reverse_iterator tmp(*this);
            --current_;
            return tmp;
        }

        constexpr reverse_iterator& operator--()
        {
            ++current_;
            return *this;
        }

        constexpr reverse_iterator operator--(int)
        {
            reverse_iterator tmp(*this);
            ++current_;
            return tmp;
        }

        constexpr bool operator==(const reverse_iterator& other) const
        {
            return current_ == other.current_;
        }

        constexpr bool operator!=(const reverse_iterator& other) const
        {
            return current_ != other.current_;
        }
    };

}

#pragma once

#include "utility.hpp"
#include <cstddef>

namespace mystl 
{
    // Default comparator
    template <typename T>
    struct less 
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const 
        {
            return lhs < rhs;
        }
    };

    template <typename T>
    struct greater
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const 
        {
            return lhs > rhs;
        }
    };

    // Helper function for sift-down (O(log N))
    template <typename RandomIt, typename Distance, typename Compare>
    void sift_down(RandomIt first, Distance len, Distance start, Compare comp) 
    {
        Distance parent = start;
        // Remember the sifted element (via move semantics)
        auto value = mystl::move(*(first + parent)); 

        while (true) 
        {
            Distance child = 2 * parent + 1; // Left child
            if (child >= len) 
                break;

            // Select the larger child
            if (child + 1 < len && comp(*(first + child), *(first + child + 1))) 
            {
                child++;
            }

            // If the parent is smaller than the child, move the child up
            if (comp(value, *(first + child))) 
            {
                *(first + parent) = mystl::move(*(first + child));
                parent = child;
            } 
            else 
            {
                break;
            }
        }
        // Place the element in its rightful position
        *(first + parent) = mystl::move(value); 
    }

    template <typename RandomIt, typename Compare>
    void push_heap(RandomIt first, RandomIt last, Compare comp) 
    {
        using Distance = std::ptrdiff_t;
        Distance len = last - first;
        if (len < 2) 
            return;

        Distance child = len - 1;
        auto value = mystl::move(*(last - 1));

        // Sift-up (O(log N))
        while (child > 0) 
        {
            Distance parent = (child - 1) / 2;
            if (comp(*(first + parent), value)) 
            {
                *(first + child) = mystl::move(*(first + parent));
                child = parent;
            } 
            else 
            {
                break;
            }
        }
        *(first + child) = mystl::move(value);
    }

    template <typename RandomIt, typename Compare>
    void pop_heap(RandomIt first, RandomIt last, Compare comp) 
    {
        if (last - first < 2) return;
        // Swap the root (first element) with the last one
        mystl::swap(*first, *(last - 1));
        // Restore the heap property for the remaining elements
        sift_down(first, static_cast<std::ptrdiff_t>(last - first - 1), static_cast<std::ptrdiff_t>(0), comp);
    }

    template <typename RandomIt, typename Compare>
    void make_heap(RandomIt first, RandomIt last, Compare comp) 
    {
        using Distance = std::ptrdiff_t;
        Distance len = last - first;
        if (len < 2) 
            return;

        // Start from the last node that has children and move toward the root (O(N))
        for (Distance i = (len - 2) / 2; i >= 0; --i) 
        {
            sift_down(first, len, i, comp);
        }
    }

} // namespace mystl
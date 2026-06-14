#pragma once

#include "deque.hpp"
#include <utility>

namespace mystl 
{

    template <typename T, typename Container = mystl::Deque<T>>
    class Queue 
    {
    protected:
        Container c_; // Underlying container (by default our Deque)

    public:
        using container_type  = Container;
        using value_type      = typename Container::value_type;
        using size_type       = typename Container::size_type;
        using reference       = typename Container::reference;
        using const_reference = typename Container::const_reference;

        // ========================================================================
        // CONSTRUCTORS
        // ========================================================================
        
        Queue() : c_() {}
        explicit Queue(const Container& cont) : c_(cont) {}
        explicit Queue(Container&& cont) : c_(mystl::move(cont)) {}

        // ========================================================================
        // ELEMENT ACCESS
        // ========================================================================
        
        [[nodiscard]] reference front() { return c_.front(); }
        [[nodiscard]] const_reference front() const { return c_.front(); }
        
        [[nodiscard]] reference back() { return c_.back(); }
        [[nodiscard]] const_reference back() const { return c_.back(); }

        // ========================================================================
        // CAPACITY
        // ========================================================================
        
        [[nodiscard]] bool empty() const { return c_.empty(); }
        [[nodiscard]] size_type size() const { return c_.size(); }

        // ========================================================================
        // MODIFIERS
        // ========================================================================
        
        void push(const value_type& value) 
        { 
            c_.push_back(value); 
        }
        
        void push(value_type&& value) 
        { 
            c_.push_back(mystl::move(value)); 
        }

        template <typename... Args>
        decltype(auto) emplace(Args&&... args) 
        { 
            return c_.emplace_back(mystl::forward<Args>(args)...); 
        }

        void pop() 
        { 
            c_.pop_front(); 
        }

        void swap(Queue& other) noexcept(noexcept(mystl::swap(c_, other.c_))) 
        {
            using mystl::swap;
            swap(c_, other.c_);
        }
    };

} // namespace mystl
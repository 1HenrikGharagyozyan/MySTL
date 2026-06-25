#pragma once

#include "deque.hpp"
#include "utility.hpp"
#include "type_traits.hpp"

namespace mystl 
{
    // By default, mystl::Deque is used, but List can be passed instead
    template <typename T, typename Container = mystl::Deque<T>>
    class Queue 
    {
    protected:
        Container c_; // Internal container

    public:
        using container_type  = Container;
        using value_type      = typename Container::value_type;
        using size_type       = typename Container::size_type;
        using reference       = typename Container::reference;
        using const_reference = typename Container::const_reference;
        using allocator_type  = typename Container::allocator_type;

        static_assert(mystl::is_same_v<T, value_type>,
                      "Queue<T, Container>: Container::value_type must be T");

        // ========================================================================
        // CONSTRUCTORS
        // ========================================================================
        Queue() : c_() {}
        explicit Queue(const allocator_type& alloc) : c_(alloc) {}
        explicit Queue(const Container& cont) : c_(cont) {}
        explicit Queue(Container&& cont) : c_(mystl::move(cont)) {}
        Queue(const Container& cont, const allocator_type& alloc) : c_(cont, alloc) {}
        Queue(Container&& cont, const allocator_type& alloc) : c_(mystl::move(cont), alloc) {}
        Queue(const Queue& other, const allocator_type& alloc) : c_(other.c_, alloc) {}
        Queue(Queue&& other, const allocator_type& alloc) : c_(mystl::move(other.c_), alloc) {}

        Queue(const Queue& other) = default;
        Queue(Queue&& other) noexcept = default;
        Queue& operator=(const Queue& other) = default;
        Queue& operator=(Queue&& other) noexcept = default;
        ~Queue() = default;

        // ========================================================================
        // ELEMENT ACCESS
        // ========================================================================
        [[nodiscard]] reference front() { return c_.front(); }
        [[nodiscard]] const_reference front() const { return c_.front(); }
        
        [[nodiscard]] reference back() { return c_.back(); }
        [[nodiscard]] const_reference back() const { return c_.back(); }

        // ========================================================================
        // SIZE
        // ========================================================================
        [[nodiscard]] bool empty() const { return c_.empty(); }
        [[nodiscard]] size_type size() const { return c_.size(); }
        [[nodiscard]] allocator_type get_allocator() const noexcept { return c_.get_allocator(); }

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
            mystl::swap(c_, other.c_);
        }
    };

    // Global swap function for ADL support
    template <typename T, typename Container>
    void swap(Queue<T, Container>& lhs, Queue<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

} // namespace mystl
#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <initializer_list>
#include <memory>
#include "allocator.hpp"
#include "utility.hpp"

namespace mystl 
{

    template <typename T, typename Allocator = mystl::Allocator<T>>
    class ForwardList 
    {
    private:
        // Base node: stores only a pointer (solves the dummy-node problem)
        struct NodeBase 
        {
            NodeBase* next;
            NodeBase() noexcept 
                : next(nullptr) 
            {
            }
        };

        // Node with data
        struct Node : public NodeBase 
        {
            T data;
            template<typename... Args>
            explicit Node(Args&&... args) 
                : data(mystl::forward<Args>(args)...) 
            {
            }
        };

        NodeBase head_; // Dummy node (before_begin). head_.next is the first real element.

        using allocator_traits = std::allocator_traits<Allocator>;
        using node_allocator_type = typename allocator_traits::template rebind_alloc<Node>;
        node_allocator_type alloc_;

    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        // ========================================================================
        // ITERATORS (Forward Only)
        // ========================================================================
        class ConstIterator;

        class Iterator 
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using pointer           = T*;
            using reference         = T&;

        private:
            NodeBase* node_;
            friend class ForwardList;
            friend class ConstIterator;

            explicit Iterator(NodeBase* ptr) noexcept : node_(ptr) {}

        public:
            Iterator() noexcept : node_(nullptr) {}

            reference operator*() const noexcept { return static_cast<Node*>(node_)->data; }
            pointer operator->() const noexcept { return &(static_cast<Node*>(node_)->data); }

            Iterator& operator++() noexcept { node_ = node_->next; return *this; }
            Iterator operator++(int) noexcept { Iterator tmp = *this; node_ = node_->next; return tmp; }

            bool operator==(const Iterator& rhs) const noexcept { return node_ == rhs.node_; }
            bool operator!=(const Iterator& rhs) const noexcept { return node_ != rhs.node_; }
            bool operator==(const ConstIterator& rhs) const noexcept;
        };

        class ConstIterator 
        {
        private:
            const NodeBase* node_;
            friend class ForwardList;

            explicit ConstIterator(const NodeBase* ptr) noexcept : node_(ptr) {}

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const T*;
            using reference         = const T&;

            ConstIterator() noexcept : node_(nullptr) {}
            ConstIterator(const Iterator& other) noexcept : node_(other.node_) {}

            const_reference operator*() const noexcept { return static_cast<const Node*>(node_)->data; }
            const_pointer operator->() const noexcept { return &(static_cast<const Node*>(node_)->data); }

            ConstIterator& operator++() noexcept { node_ = node_->next; return *this; }
            ConstIterator operator++(int) noexcept { ConstIterator tmp = *this; node_ = node_->next; return tmp; }

            bool operator==(const ConstIterator& rhs) const noexcept { return node_ == rhs.node_; }
            bool operator!=(const ConstIterator& rhs) const noexcept { return node_ != rhs.node_; }
        };

        using iterator = Iterator;
        using const_iterator = ConstIterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
	    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // ========================================================================
        // ACCESS METHODS
        // ========================================================================
        iterator before_begin() noexcept { return iterator(&head_); }
        const_iterator before_begin() const noexcept { return const_iterator(&head_); }
        const_iterator cbefore_begin() const noexcept { return const_iterator(&head_); }

        iterator begin() noexcept { return iterator(head_.next); }
        iterator end() noexcept { return iterator(nullptr); }
        const_iterator begin() const noexcept { return const_iterator(head_.next); }
        const_iterator end() const noexcept { return const_iterator(nullptr); }
        const_iterator cbegin() const noexcept { return const_iterator(head_.next); }
        const_iterator cend() const noexcept { return const_iterator(nullptr); }

        reverse_iterator rbegin() { return reverse_iterator(end()); }
        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
        const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

        bool empty() const noexcept { return head_.next == nullptr; }

        reference front() { return *begin(); }
        const_reference front() const { return *begin(); }

        // ========================================================================
        // CONSTRUCTORS AND RULE OF FIVE
        // ========================================================================
        ForwardList() noexcept = default;
        ~ForwardList();
        ForwardList(const ForwardList& other);
        ForwardList(ForwardList&& other) noexcept;
        ForwardList(std::initializer_list<value_type> init);

        ForwardList& operator=(const ForwardList& other);
        ForwardList& operator=(ForwardList&& other) noexcept;

        // ========================================================================
        // MODIFIERS
        // ========================================================================
        void swap(ForwardList& other) noexcept { mystl::swap(head_.next, other.head_.next); }

        void clear() noexcept;

        template <typename... Args>
        iterator emplace_after(const_iterator pos, Args&&... args);

        iterator insert_after(const_iterator pos, const T& value) { return emplace_after(pos, value); }
        iterator insert_after(const_iterator pos, T&& value) { return emplace_after(pos, mystl::move(value)); }

        iterator erase_after(const_iterator pos);

        void push_front(const T& value) { insert_after(before_begin(), value); }
        void push_front(T&& value) { insert_after(before_begin(), mystl::move(value)); }

        void pop_front() { erase_after(before_begin()); }

        // ========================================================================
        // ALGORITHMS (Zero-Allocation Pointer Manipulation)
        // ========================================================================

        // Reverse the list in O(N) time and O(1) extra memory
        void reverse() noexcept;

        // Remove consecutive duplicates in O(N)
        void unique();

        // Merge two sorted lists in O(N)
        void merge(ForwardList& other) noexcept;

        // Merge sort (bottom-up merge sort) in O(N log N)
        void sort();

    private:
        // Helpers for memory management (to avoid duplicating try/catch)
        template <typename... Args>
        Node* create_node(Args&&... args) 
        {
            Node* new_node = alloc_.allocate(1);
            try 
            {
                alloc_.construct(new_node, mystl::forward<Args>(args)...);
            } 
            catch (...) 
            {
                alloc_.deallocate(new_node, 1);
                throw;
            }
            return new_node;
        }

        void destroy_node(Node* node) noexcept 
        {
            alloc_.destroy(node);
            alloc_.deallocate(node, 1);
        }
    };


    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>::~ForwardList()
    {
        clear();
    }

    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>::ForwardList(const ForwardList& other)
    {
        NodeBase* current = &head_;
        for (const auto& value : other) 
        {
            current->next = create_node(value);
            current = current->next;
        }
    }

    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>::ForwardList(ForwardList&& other) noexcept
        : alloc_(mystl::move(other.alloc_)) 
    {
        head_.next = other.head_.next;
        other.head_.next = nullptr;
    }

    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>::ForwardList(std::initializer_list<value_type> init)
    {
        NodeBase* current = &head_;
        for (const auto& value : init) 
        {
            current->next = create_node(value);
            current = current->next;
        }
    }

    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>& ForwardList<T, Allocator>::operator=(const ForwardList& other)
    {
        if (this != &other) 
        {
            ForwardList temp(other);
            swap(temp);
        }
        return *this;
    }

    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>& ForwardList<T, Allocator>::operator=(ForwardList&& other) noexcept
    {
        if (this != &other) 
        {
            clear();
            alloc_ = mystl::move(other.alloc_);
            head_.next = other.head_.next;
            other.head_.next = nullptr;
        }
        return *this;
    }

    template<typename T, typename Allocator>
    inline void ForwardList<T, Allocator>::clear() noexcept
    {
        NodeBase* current = head_.next;
        while (current) 
        {
            NodeBase* next = current->next;
            destroy_node(static_cast<Node*>(current));
            current = next;
        }
        head_.next = nullptr;
    }

    template<typename T, typename Allocator>
    inline ForwardList<T, Allocator>::iterator ForwardList<T, Allocator>::erase_after(const_iterator pos)
    {
        NodeBase* current = const_cast<NodeBase*>(pos.node_);
        NodeBase* node_to_erase = current->next;
        
        if (node_to_erase) 
        {
            current->next = node_to_erase->next;
            destroy_node(static_cast<Node*>(node_to_erase));
        }
        return iterator(current->next);
    }

    template<typename T, typename Allocator>
    inline void ForwardList<T, Allocator>::reverse() noexcept
    {
        NodeBase* prev = nullptr;
        NodeBase* current = head_.next;
        
        while (current) 
        {
            NodeBase* next_node = current->next;
            current->next = prev;
            prev = current;
            current = next_node;
        }
        head_.next = prev;
    }

    template<typename T, typename Allocator>
    inline void ForwardList<T, Allocator>::unique()
    {
        NodeBase* current = head_.next;
        if (!current) 
            return;

        while (current->next) 
        {
            Node* curr_node = static_cast<Node*>(current);
            Node* next_node = static_cast<Node*>(current->next);

            if (curr_node->data == next_node->data) 
            {
                NodeBase* duplicate = current->next;
                current->next = duplicate->next;
                destroy_node(static_cast<Node*>(duplicate));
            } 
            else 
            {
                current = current->next;
            }
        }
    }

    template<typename T, typename Allocator>
    inline void ForwardList<T, Allocator>::merge(ForwardList& other) noexcept
    {
        if (this == &other) 
            return;

        NodeBase* a = head_.next;
        NodeBase* b = other.head_.next;
        NodeBase* tail = &head_; // Build directly on our dummy node

        while (a && b) 
        {
            // Use the < operator for comparison
            if (static_cast<Node*>(b)->data < static_cast<Node*>(a)->data) 
            {
                tail->next = b;
                b = b->next;
            } 
            else 
            {
                tail->next = a;
                a = a->next;
            }
            tail = tail->next;
        }
        
        // Attach the remaining nodes
        tail->next = a ? a : b;
        other.head_.next = nullptr;
    }

    template<typename T, typename Allocator>
    inline void ForwardList<T, Allocator>::sort()
    {
        if (!head_.next || !head_.next->next) 
            return;

        size_type list_size = 1;
        while (true) 
        {
            NodeBase* current = head_.next;
            head_.next = nullptr;
            NodeBase* tail = &head_;
            size_type merges_done = 0;

            while (current) 
            {
                NodeBase* left = current;
                NodeBase* right = nullptr;
                size_type left_size = 0;
                size_type right_size = 0;

                // Measure out the left part
                for (size_type i = 0; i < list_size && current; ++i) 
                {
                    ++left_size;
                    current = current->next;
                }

                right = current;

                // Measure out the right part
                for (size_type i = 0; i < list_size && current; ++i) 
                {
                    ++right_size;
                    current = current->next;
                }

                // Merge left and right
                while (left_size > 0 || right_size > 0) 
                {
                    if (left_size == 0) 
                    {
                        tail->next = right; right = right->next; --right_size;
                    } 
                    else if (right_size == 0) 
                    {
                        tail->next = left; left = left->next; --left_size;
                    } 
                    else if (static_cast<Node*>(right)->data < static_cast<Node*>(left)->data) 
                    {
                        tail->next = right; right = right->next; --right_size;
                    } 
                    else 
                    {
                        tail->next = left; left = left->next; --left_size;
                    }
                    tail = tail->next;
                }
                tail->next = nullptr; // Close the tail
                ++merges_done;
            }

            // If only one run was produced in this pass, the list is sorted
            if (merges_done <= 1) break;
            
            list_size *= 2;
        }
    }

    template<typename T, typename Allocator>
    template<typename ...Args>
    inline ForwardList<T, Allocator>::iterator ForwardList<T, Allocator>::emplace_after(const_iterator pos, Args && ...args)
    {
        Node* new_node = create_node(mystl::forward<Args>(args)...);
        NodeBase* current = const_cast<NodeBase*>(pos.node_);
        
        new_node->next = current->next;
        current->next = new_node;
        
        return iterator(new_node);
    }
    


    template <typename T, typename Allocator>
    inline bool ForwardList<T, Allocator>::Iterator::operator==(const ConstIterator& rhs) const noexcept 
    {
        return node_ == rhs.node_;
    }

    template <typename T, typename Allocator>
    bool operator==(const ForwardList<T, Allocator>& lhs, const ForwardList<T, Allocator>& rhs) 
    {
        auto it1 = lhs.begin();
        auto it2 = rhs.begin();
        while (it1 != lhs.end() && it2 != rhs.end()) 
        {
            if (*it1 != *it2) return false;
            ++it1;
            ++it2;
        }
        return it1 == lhs.end() && it2 == rhs.end(); // Verify that both lists ended at the same time
    }

    template <typename T, typename Allocator>
    bool operator!=(const ForwardList<T, Allocator>& lhs, const ForwardList<T, Allocator>& rhs) 
    {
        return !(lhs == rhs);
    }



} // namespace mystl
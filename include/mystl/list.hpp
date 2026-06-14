#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <initializer_list>
#include <type_traits>
#include <memory>

#include "allocator.hpp"
#include "utility.hpp"

namespace mystl 
{

    template <typename T, typename Allocator = mystl::Allocator<T>>
    class List 
    {
    private:
        struct NodeBase 
        {
            NodeBase* prev;
            NodeBase* next;
            
            NodeBase() noexcept 
                : prev(this)
                , next(this) 
            {
            } 
        };

        struct Node : public NodeBase 
        {
            T data;
            
            template<typename... Args>
            explicit Node(Args&&... args) 
                : data(mystl::forward<Args>(args)...) 
            {
            }
        };

        NodeBase sentinel_;
        std::size_t size_ = 0;
        
        using allocator_traits = std::allocator_traits<Allocator>;        
        using node_allocator_type = typename allocator_traits::template rebind_alloc<Node>;
        
        node_allocator_type alloc_;

    public:
        using allocator_type = Allocator;
        using value_type = T;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        // ========================================================================
        // ITERATORS (Strictly Bidirectional)
        // ========================================================================
        class ConstIterator;

        class Iterator 
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using pointer           = T*;
            using reference         = T&;

        private:
            NodeBase* node_;
            friend class List;
            friend class ConstIterator;

            explicit Iterator(NodeBase* ptr) noexcept : node_(ptr) {}

        public:
            Iterator() noexcept : node_(nullptr) {}

            reference operator*() const noexcept { return static_cast<Node*>(node_)->data; }
            pointer operator->() const noexcept { return &(static_cast<Node*>(node_)->data); }

            Iterator& operator++() noexcept { node_ = node_->next; return *this; }
            Iterator operator++(int) noexcept { Iterator tmp = *this; ++(*this); return tmp; }

            Iterator& operator--() noexcept { node_ = node_->prev; return *this; }
            Iterator operator--(int) noexcept { Iterator tmp = *this; --(*this); return tmp; }

            bool operator==(const Iterator& rhs) const noexcept { return node_ == rhs.node_; }
            bool operator!=(const Iterator& rhs) const noexcept { return node_ != rhs.node_; }
            
            // Cross-comparisons with ConstIterator
            bool operator==(const ConstIterator& rhs) const noexcept;
        };

        class ConstIterator 
        {
            // ... Analogous to Iterator, but with const types and a constructor from Iterator
        private:
            const NodeBase* node_;
            friend class List;

            explicit ConstIterator(const NodeBase* ptr) noexcept : node_(ptr) {}

        public:
            ConstIterator() noexcept : node_(nullptr) {}
            ConstIterator(const Iterator& other) noexcept : node_(other.node_) {}

            const_reference operator*() const noexcept { return static_cast<const Node*>(node_)->data; }
            const_pointer operator->() const noexcept { return &(static_cast<const Node*>(node_)->data); }

            ConstIterator& operator++() noexcept { node_ = node_->next; return *this; }
            ConstIterator operator++(int) noexcept { ConstIterator tmp = *this; ++(*this); return tmp; }

            ConstIterator& operator--() noexcept { node_ = node_->prev; return *this; }
            ConstIterator operator--(int) noexcept { ConstIterator tmp = *this; --(*this); return tmp; }

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
        iterator begin() noexcept { return iterator(sentinel_.next); }
        iterator end() noexcept { return iterator(&sentinel_); }
        const_iterator begin() const noexcept { return const_iterator(sentinel_.next); }
        const_iterator end() const noexcept { return const_iterator(&sentinel_); }
        const_iterator cbegin() const noexcept { return const_iterator(sentinel_.next); }
        const_iterator cend() const noexcept { return const_iterator(&sentinel_); }

        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
        const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
        const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

        bool empty() const noexcept { return size_ == 0; }
        size_type size() const noexcept { return size_; }

        reference front() { return *begin(); }
        reference back() { return *(--end()); } // Sentinel magic: --end() always works!
        const_reference front() const { return *begin(); }
        const_reference back() const { return *(--end()); }

        // ========================================================================
        // CONSTRUCTORS AND RULE OF FIVE
        // ========================================================================
        List() noexcept = default;
        ~List();
        List(const List& other);
        List(List&& other) noexcept;
        List(std::initializer_list<value_type> init);

        List& operator=(const List& other);
        List& operator=(List&& other) noexcept;


        // ========================================================================
        // MODIFIERS
        // ========================================================================
        template <typename... Args>
        iterator emplace(const_iterator pos, Args&&... args);

        void push_back(const T& value) { emplace(end(), value); }
        void push_back(T&& value) { emplace(end(), mystl::move(value)); }

        void push_front(const T& value) { emplace(begin(), value); }
        void push_front(T&& value) { emplace(begin(), mystl::move(value)); }

        iterator insert(const_iterator pos, const T& value) { return emplace(pos, value); }
        iterator erase(const_iterator pos);
        
        void pop_back() { if (size_ > 0) erase(--end()); }
        void pop_front() { if (size_ > 0) erase(begin()); }
        
        void clear() noexcept;

        void swap(List& other) noexcept;

        // ========================================================================
        // ALGORITHMS FOR A DOUBLY LINKED LIST (Zero-Allocation Pointer Manipulation)
        // ========================================================================

        // Reverse a doubly linked list in O(N) time and O(1) extra memory
        void reverse() noexcept;

        // Remove consecutive duplicates in O(N)
        void unique();

        // Merge two sorted lists in O(N)
        void merge(List& other);

        // Merge sort (sorts in O(N log N) time and O(1) extra memory)
        // This is the production-grade implementation from GCC STL
        void sort();

    private:
        // Helper: links two nodes together (used during insertion and removal)
        void link_nodes(NodeBase* prev, NodeBase* next) noexcept 
        {
            prev->next = next;
            next->prev = prev;
        }
    };

    template <typename T, typename Allocator>
    inline bool List<T, Allocator>::Iterator::operator==(const ConstIterator& rhs) const noexcept 
    {
        return node_ == rhs.node_;
    }

    // ========================================================================
    // TEMPLATE METHOD IMPLEMENTATION
    // ========================================================================

    // Destructor
    template <typename T, typename Allocator>
    List<T, Allocator>::~List() 
    {
        clear();
    }

    // Copy Constructor
    template <typename T, typename Allocator>
    List<T, Allocator>::List(const List& other) 
    {
        for (const auto& value : other) 
            push_back(value);
    }

    // Move Constructor (Stealing pointers in O(1))
    template <typename T, typename Allocator>
    List<T, Allocator>::List(List&& other) noexcept 
        : size_(other.size_)
        , alloc_(mystl::move(other.alloc_)) 
    {
        if (size_ > 0) 
        {
            // Steal the links from the other list's sentinel_
            sentinel_.next = other.sentinel_.next;
            sentinel_.prev = other.sentinel_.prev;
            
            // Connect the stolen nodes to our sentinel_
            sentinel_.next->prev = &sentinel_;
            sentinel_.prev->next = &sentinel_;
            
            // Put other into a valid empty state
            other.sentinel_.next = &other.sentinel_;
            other.sentinel_.prev = &other.sentinel_;
            other.size_ = 0;
        }
    }

    // Initializer list
    template <typename T, typename Allocator>
    List<T, Allocator>::List(std::initializer_list<value_type> init) 
    {
        for (const auto& value : init) 
            push_back(value);
    }

    // Copy Assignment (Copy-and-Swap idiom for strong exception safety)
    template <typename T, typename Allocator>
    List<T, Allocator>& List<T, Allocator>::operator=(const List& other) 
    {
        if (this != &other) 
        {
            List temp(other); // Create a temporary copy
            swap(temp);       // Swap contents with the temporary copy (safe with exceptions)
        }
        return *this;
    }

    // Move Assignment
    template <typename T, typename Allocator>
    List<T, Allocator>& List<T, Allocator>::operator=(List&& other) noexcept 
    {
        if (this != &other) 
        {
            clear();
            swap(other); // Just swap the contents (safe with exceptions)
        }
        return *this;
    }

    // Clear: Destroy all nodes, leaving only the sentinel
    template <typename T, typename Allocator>
    void List<T, Allocator>::clear() noexcept 
    {
        NodeBase* current = sentinel_.next;
        while (current != &sentinel_) 
        {
            NodeBase* next_node = current->next;
            Node* node_to_delete = static_cast<Node*>(current);
            
            alloc_.destroy(node_to_delete);
            alloc_.deallocate(node_to_delete, 1);
            
            current = next_node;
        }
        
        sentinel_.next = &sentinel_;
        sentinel_.prev = &sentinel_;
        size_ = 0;
    }

    template<typename T, typename Allocator>
    inline void List<T, Allocator>::swap(List & other) noexcept
    {
        mystl::swap(size_, other.size_);

        mystl::swap(sentinel_.next, other.sentinel_.next);
        mystl::swap(sentinel_.prev, other.sentinel_.prev);

        // Reconnect the "stolen" nodes to the new sentinels
        if (size_ > 0) 
        {
            sentinel_.next->prev = &sentinel_;
            sentinel_.prev->next = &sentinel_;
        } 
        else 
        {
            sentinel_.next = sentinel_.prev = &sentinel_;
        }

        if (other.size_ > 0) 
        {
            other.sentinel_.next->prev = &other.sentinel_;
            other.sentinel_.prev->next = &other.sentinel_;
        } 
        else 
        {
            other.sentinel_.next = other.sentinel_.prev = &other.sentinel_;
        }
    }

    template<typename T, typename Allocator>
    inline void List<T, Allocator>::reverse() noexcept
    {
        if (size_ <= 1) 
            return;

        NodeBase* current = &sentinel_;
        do {
            // In each node (including the sentinel), simply swap next and prev.
            mystl::swap(current->next, current->prev);

            // Since we just swapped them, the old next is now in prev.
            // Move to the next node by following the prev pointer.
            current = current->prev; 
        } while (current != &sentinel_);
    }

    template<typename T, typename Allocator>
    inline void List<T, Allocator>::unique()
    {
        if (size_ <= 1) 
            return;

        iterator it = begin();
        iterator next_it = it;
        ++next_it;

        while (next_it != end()) 
        {
            if (*it == *next_it) 
            {
                next_it = erase(next_it); // erase will itself decrement size_ and remove the node
            } 
            else 
            {
                it = next_it;
                ++next_it;
            }
        }
    }

    template<typename T, typename Allocator>
    inline void List<T, Allocator>::merge(List& other)
    {
        if (this == &other || other.empty()) 
            return;

        iterator it1 = begin();
        iterator it2 = other.begin();

        while (it1 != end() && it2 != other.end()) 
        {
            if (*it2 < *it1) 
            {
                // Remove the node from the other list (without allocations!)
                NodeBase* node_to_move = it2.node_;
                ++it2;

                other.link_nodes(node_to_move->prev, node_to_move->next);
                --other.size_;

                // Insert the node into our list directly before it1
                NodeBase* curr = it1.node_;
                NodeBase* prev_node = curr->prev;

                link_nodes(prev_node, node_to_move);
                link_nodes(node_to_move, curr);
                ++size_;
            } 
            else 
            {
                ++it1;
            }
        }

        // If other still has elements, move its remaining portion to the end of our list
        if (!other.empty()) 
        {
            NodeBase* first_rem = other.sentinel_.next;
            NodeBase* last_rem = other.sentinel_.prev;
            NodeBase* my_last = sentinel_.prev;

            link_nodes(my_last, first_rem);
            link_nodes(last_rem, &sentinel_);

            size_ += other.size_;

            // Put other back into a valid empty state
            other.sentinel_.next = &other.sentinel_;
            other.sentinel_.prev = &other.sentinel_;
            other.size_ = 0;
        }
    }

    template<typename T, typename Allocator>
    inline void List<T, Allocator>::sort()
    {
        if (size_ <= 1) 
            return;

        // An array of temporary list buffers for bottom-up merge sort
        // 64 buffers are enough for 2^64 elements
        List carry;
        List counter[64];
        int fill = 0;

        while (!empty()) 
        {
            // Detach the first node from our list and move it to carry
            NodeBase* node = sentinel_.next;
            link_nodes(&sentinel_, node->next);
            --size_;

            carry.link_nodes(&carry.sentinel_, node);
            carry.link_nodes(node, &carry.sentinel_);
            carry.size_ = 1;

            int i = 0;
            while (i < fill && !counter[i].empty()) 
            {
                counter[i].merge(carry);
                carry.swap(counter[i]);
                ++i;
            }
            carry.swap(counter[i]);
            if (i == fill) ++fill;
        }

        // Reassemble all runs back together
        for (int i = 0; i < fill; ++i) 
        {
            merge(counter[i]);
        }
    }

    // Emplace: Efficient insertion
    template <typename T, typename Allocator>
    template <typename... Args>
    typename List<T, Allocator>::iterator List<T, Allocator>::emplace(const_iterator pos, Args&&... args) 
    {
        Node* new_node = alloc_.allocate(1);
        try 
        {
            alloc_.construct(new_node, mystl::forward<Args>(args)...);
        } 
        catch (...) 
        {
            alloc_.deallocate(new_node, 1);
            throw; // Strong exception guarantee
        }

        NodeBase* current = const_cast<NodeBase*>(pos.node_);
        NodeBase* prev = current->prev;

        link_nodes(prev, new_node);
        link_nodes(new_node, current);

        ++size_;
        return iterator(new_node);
    }

    // Erase: Efficient removal
    template <typename T, typename Allocator>
    typename List<T, Allocator>::iterator List<T, Allocator>::erase(const_iterator pos) 
    {
        NodeBase* node_to_erase = const_cast<NodeBase*>(pos.node_);
        NodeBase* next_node = node_to_erase->next;
        
        // Remove the node from the ring
        link_nodes(node_to_erase->prev, node_to_erase->next);
        
        // Destroy the object and release the memory
        Node* node = static_cast<Node*>(node_to_erase);
        alloc_.destroy(node);
        alloc_.deallocate(node, 1);
        
        --size_;
        return iterator(next_node);
    }

    // ========================================================================
    // GLOBAL OPERATORS AND FUNCTIONS
    // ========================================================================

    template <typename T, typename Allocator>
    bool operator==(const List<T, Allocator>& lhs, const List<T, Allocator>& rhs) 
    {
        if (lhs.size() != rhs.size()) return false;
        auto it1 = lhs.begin();
        auto it2 = rhs.begin();
        while (it1 != lhs.end()) 
        {
            if (*it1 != *it2) 
                return false;
            ++it1;
            ++it2;
        }
        return true;
    }

    template <typename T, typename Allocator>
    bool operator!=(const List<T, Allocator>& lhs, const List<T, Allocator>& rhs) 
    {
        return !(lhs == rhs);
    }

    template <typename T, typename Allocator>
    void swap(List<T, Allocator>& lhs, List<T, Allocator>& rhs) noexcept 
    {
        lhs.swap(rhs);
    }

} // namespace mystl
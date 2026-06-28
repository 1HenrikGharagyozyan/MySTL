#pragma once

#include "rb_tree.hpp"
#include "allocator.hpp"
#include "utility.hpp"
#include "functional.hpp" 

#include <initializer_list>
#include <stdexcept>

namespace mystl 
{
    template <
        typename Key, 
        typename T,
        typename Compare = mystl::less, 
        typename Allocator = mystl::Allocator<mystl::Pair<const Key, T>>
    >
    class Map 
    {
    public:
        using key_type               = Key;
        using mapped_type            = T;
        using value_type             = mystl::Pair<const Key, T>;
        using key_compare            = Compare;
        
        class value_compare 
        {
            friend class Map;
        protected:
            Compare comp;
            value_compare(Compare c) : comp(c) {}
        public:
            bool operator()(const value_type& lhs, const value_type& rhs) const 
            {
                return comp(lhs.first, rhs.first);
            }
        };

    private:
        using Tree = RBTree<Key, value_type, mystl::Select1st<value_type>, Compare, Allocator>;
        Tree tree_;

    public:
        using size_type              = typename Tree::size_type;
        using difference_type        = typename Tree::difference_type;
        using allocator_type         = Allocator;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        using pointer                = typename mystl::allocator_traits<Allocator>::pointer;
        using const_pointer          = typename mystl::allocator_traits<Allocator>::const_pointer;
        
        using iterator               = typename Tree::iterator;
        using const_iterator         = typename Tree::const_iterator;
        using reverse_iterator       = typename Tree::reverse_iterator;
        using const_reverse_iterator = typename Tree::const_reverse_iterator;

        // ========================================================================
        // CONSTRUCTORS
        // ========================================================================
        
        Map() : tree_() {}
        
        explicit Map(const Compare& comp, const Allocator& alloc = Allocator()) 
            : tree_(comp, alloc) 
        {
        }
            
        explicit Map(const Allocator& alloc) 
            : tree_(Compare(), alloc) 
        {
        }

        template <typename InputIt>
        Map(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : tree_(comp, alloc) 
        {
            for (; first != last; ++first) 
            {
                tree_.emplace_unique(*first);
            }
        }

        Map(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : tree_(comp, alloc)
        {
            for (const auto& val : init) 
            {
                tree_.emplace_unique(val);
            }
        }

        // ========================================================================
        // RULE OF FIVE
        // ========================================================================
        
        Map(const Map& other) : tree_(other.tree_) {}
        Map(Map&& other) noexcept : tree_(mystl::move(other.tree_)) {}
        
        ~Map() = default;

        Map& operator=(const Map& other) 
        { 
            tree_ = other.tree_; 
            return *this; 
        }
        
        Map& operator=(Map&& other) noexcept 
        { 
            tree_ = mystl::move(other.tree_); 
            return *this; 
        }

        // ========================================================================
        // ELEMENT ACCESS
        // ========================================================================
        
        mapped_type& operator[](const key_type& key) 
        {
            return try_emplace(key).first->second;
        }

        mapped_type& operator[](key_type&& key) 
        {
            return try_emplace(mystl::move(key)).first->second;
        }

        mapped_type& at(const key_type& key) 
        {
            iterator it = tree_.find(key);
            if (it == end()) 
                throw std::out_of_range("mystl::map::at: key not found");
            return it->second;
        }

        const mapped_type& at(const key_type& key) const 
        {
            const_iterator it = tree_.find(key);
            if (it == cend()) 
                throw std::out_of_range("mystl::map::at: key not found");
            return it->second;
        }

        // ========================================================================
        // CAPACITY & OBSERVERS
        // ========================================================================
        
        [[nodiscard]] allocator_type get_allocator() const noexcept { return tree_.get_allocator(); }
        [[nodiscard]] bool empty() const noexcept { return tree_.empty(); }
        [[nodiscard]] size_type size() const noexcept { return tree_.size(); }
        
        key_compare key_comp() const { return key_compare(); }
        value_compare value_comp() const { return value_compare(key_comp()); }

        // ========================================================================
        // ITERATORS
        // ========================================================================
        
        iterator begin() noexcept { return tree_.begin(); }
        iterator end() noexcept { return tree_.end(); }
        const_iterator begin() const noexcept { return tree_.begin(); }
        const_iterator end() const noexcept { return tree_.end(); }
        
        const_iterator cbegin() const noexcept { return tree_.cbegin(); }
        const_iterator cend() const noexcept { return tree_.cend(); }

        reverse_iterator rbegin() noexcept { return tree_.rbegin(); }
        reverse_iterator rend() noexcept { return tree_.rend(); }
        const_reverse_iterator crbegin() const noexcept { return tree_.crbegin(); }
        const_reverse_iterator crend() const noexcept { return tree_.crend(); }

        // ========================================================================
        // MODIFIERS
        // ========================================================================
        
        void clear() noexcept { tree_.clear(); }

        mystl::Pair<iterator, bool> insert(const value_type& value) 
        {
            return tree_.emplace_unique(value);
        }

        mystl::Pair<iterator, bool> insert(value_type&& value) 
        {
            return tree_.emplace_unique(mystl::move(value));
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last) 
        {
            for (; first != last; ++first) {
                tree_.emplace_unique(*first);
            }
        }

        void insert(std::initializer_list<value_type> ilist) 
        {
            insert(ilist.begin(), ilist.end());
        }

        template <typename... Args>
        mystl::Pair<iterator, bool> emplace(Args&&... args) 
        {
            return tree_.emplace_unique(mystl::forward<Args>(args)...);
        }

        template <typename... Args>
        mystl::Pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) 
        {
            iterator it = tree_.find(key);
            if (it != end()) 
                return {it, false};
            
            return tree_.emplace_unique(key, T(mystl::forward<Args>(args)...));
        }

        template <typename... Args>
        mystl::Pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) 
        {
            iterator it = tree_.find(key);
            if (it != end()) 
                return {it, false};
            
            return tree_.emplace_unique(mystl::move(key), T(mystl::forward<Args>(args)...));
        }

        mystl::Pair<iterator, bool> insert_or_assign(const key_type& key, const T& obj) 
        {
            iterator it = tree_.find(key);
            if (it != end()) 
            {
                it->second = obj;
                return {it, false};
            }
            return tree_.emplace_unique(key, obj);
        }

        mystl::Pair<iterator, bool> insert_or_assign(const key_type& key, T&& obj) 
        {
            iterator it = tree_.find(key);
            if (it != end()) 
            {
                it->second = mystl::move(obj);
                return {it, false};
            }
            return tree_.emplace_unique(key, mystl::move(obj));
        }

        size_type erase(const key_type& key) 
        {
            return tree_.erase(key);
        }

        void swap(Map& other) noexcept 
        {
            tree_.swap(other.tree_);
        }

        // ========================================================================
        // SEARCH
        // ========================================================================
        
        iterator find(const key_type& key) noexcept { return tree_.find(key); }
        const_iterator find(const key_type& key) const noexcept { return tree_.find(key); }

        bool contains(const key_type& key) const noexcept { return tree_.contains(key); }

        iterator lower_bound(const key_type& key) noexcept { return tree_.lower_bound(key); }
        const_iterator lower_bound(const key_type& key) const noexcept { return tree_.lower_bound(key); }

        iterator upper_bound(const key_type& key) noexcept { return tree_.upper_bound(key); }
        const_iterator upper_bound(const key_type& key) const noexcept { return tree_.upper_bound(key); }
        
        mystl::Pair<iterator, iterator> equal_range(const key_type& key) noexcept { return tree_.equal_range(key); }
        mystl::Pair<const_iterator, const_iterator> equal_range(const key_type& key) const noexcept { return tree_.equal_range(key); }
    };

    template <typename Key, typename T, typename Compare, typename Allocator>
    bool operator==(const Map<Key, T, Compare, Allocator>& lhs, 
                    const Map<Key, T, Compare, Allocator>& rhs) 
    {
        return lhs.size() == rhs.size() && mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template <typename Key, typename T, typename Compare, typename Allocator>
    bool operator!=(const Map<Key, T, Compare, Allocator>& lhs, 
                    const Map<Key, T, Compare, Allocator>& rhs) 
    {
        return !(lhs == rhs);
    }

    template <typename Key, typename T, typename Compare, typename Allocator>
    void swap(Map<Key, T, Compare, Allocator>& lhs, 
              Map<Key, T, Compare, Allocator>& rhs) noexcept 
    {
        lhs.swap(rhs);
    }

} // namespace mystl
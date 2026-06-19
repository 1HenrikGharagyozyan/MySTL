#pragma once

#include "rb_tree.hpp"
#include "allocator.hpp" 
#include "utility.hpp"

#include <initializer_list>

namespace mystl 
{
    template <
        typename Key, 
        typename Compare = std::less<Key>, 
        typename Allocator = mystl::Allocator<Key>
    >
    class Set 
    {
    private:
        // Configure RBTree: Value = Key, key extractor = Identity
        using Tree = RBTree<Key, Key, mystl::Identity<Key>, Compare, Allocator>;
        Tree tree_;

    public:
        using key_type               = Key;
        using value_type             = Key;
        using size_type              = typename Tree::size_type;
        using difference_type        = typename Tree::difference_type;
        using allocator_type         = Allocator;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        
        // In Set the iterator is always constant to prevent breaking the tree balance
        using iterator               = typename Tree::const_iterator;
        using const_iterator         = typename Tree::const_iterator;
        using reverse_iterator       = typename Tree::const_reverse_iterator;
        using const_reverse_iterator = typename Tree::const_reverse_iterator;

        // ========================================================================
        // CONSTRUCTORS
        // ========================================================================
        Set() : tree_() {}
        
        explicit Set(const Compare& comp, const Allocator& alloc = Allocator()) 
            : tree_(comp, alloc) 
        {
        }
            
        explicit Set(const Allocator& alloc) 
            : tree_(Compare(), alloc) 
        {
        }

        template <typename InputIt>
        Set(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : tree_(comp, alloc) 
        {
            for (; first != last; ++first) 
            {
                tree_.emplace_unique(*first);
            }
        }

        Set(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : tree_(comp, alloc)
        {
            for (const auto& val : init) 
            {
                tree_.emplace_unique(val);
            }
        }

        // ========================================================================
        // RULE OF FIVE AND ALLOCATORS
        // ========================================================================
        Set(const Set& other) : tree_(other.tree_) {}
        Set(const Set& other, const Allocator& alloc) : tree_(other.tree_, alloc) {}
        
        Set(Set&& other) noexcept : tree_(mystl::move(other.tree_)) {}
        Set(Set&& other, const Allocator& alloc) : tree_(mystl::move(other.tree_), alloc) {}

        Set& operator=(const Set& other) 
        { 
            tree_ = other.tree_; 
            return *this; 
        }
        
        Set& operator=(Set&& other) noexcept 
        { 
            tree_ = mystl::move(other.tree_); 
            return *this; 
        }

        // ========================================================================
        // ACCESS AND CAPACITY
        // ========================================================================
        [[nodiscard]] allocator_type get_allocator() const noexcept { return tree_.get_allocator(); }
        [[nodiscard]] bool empty() const noexcept { return tree_.empty(); }
        [[nodiscard]] size_type size() const noexcept { return tree_.size(); }
        void clear() noexcept { tree_.clear(); }

        iterator begin() const noexcept { return tree_.cbegin(); }
        iterator end() const noexcept { return tree_.cend(); }
        const_iterator cbegin() const noexcept { return tree_.cbegin(); }
        const_iterator cend() const noexcept { return tree_.cend(); }

        // ========================================================================
        // MODIFIERS
        // ========================================================================
        mystl::Pair<iterator, bool> insert(const value_type& value) 
        {
            auto res = tree_.emplace_unique(value);
            // res.first is Tree::iterator. It automatically casts to Tree::const_iterator
            return { res.first, res.second };
        }

        mystl::Pair<iterator, bool> insert(value_type&& value) 
        {
            auto res = tree_.emplace_unique(mystl::move(value));
            return { res.first, res.second };
        }

        template <typename... Args>
        mystl::Pair<iterator, bool> emplace(Args&&... args) 
        {
            auto res = tree_.emplace_unique(mystl::forward<Args>(args)...);
            return { res.first, res.second };
        }

        size_type erase(const key_type& key) 
        {
            return tree_.erase(key);
        }

        void swap(Set& other) noexcept 
        {
            tree_.swap(other.tree_);
        }

        // ========================================================================
        // SEARCH
        // ========================================================================
        iterator find(const key_type& key) const 
        {
            return tree_.find(key);
        }

        bool contains(const key_type& key) const 
        {
            return tree_.contains(key);
        }

        iterator lower_bound(const key_type& key) const 
        {
            return tree_.lower_bound(key);
        }

        iterator upper_bound(const key_type& key) const 
        {
            return tree_.upper_bound(key);
        }
    };

} // namespace mystl

// Support for allocator traits for Set
namespace std
{
    template <typename Key, typename Compare, typename Alloc>
    struct uses_allocator<mystl::Set<Key, Compare, Alloc>, Alloc> : true_type {};
}
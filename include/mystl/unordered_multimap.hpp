#pragma once

#include "utility.hpp"
#include <cstddef>
#include <functional>
#include <cassert>

namespace mystl 
{
    template <typename Key>
    struct hash_multimap 
    {
        size_t operator()(const Key& key) const 
        { 
            return std::hash<Key>{}(key); 
        }
    };

    template <typename T>
    struct equal_to_multimap 
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const 
        { 
            return lhs == rhs; 
        }
    };

    template <
        typename Key, 
        typename T,
        typename Hash = mystl::hash_multimap<Key>, 
        typename KeyEqual = mystl::equal_to_multimap<Key>
    >
    class UnorderedMultiMap 
    {
    public:
        using key_type        = Key;
        using mapped_type     = T;
        using value_type      = mystl::Pair<const Key, T>;
        using size_type       = std::size_t;
        using hasher          = Hash;
        using key_equal       = KeyEqual;
        using reference       = value_type&;
        using const_reference = const value_type&;

    private:
        struct Node 
        {
            value_type value;
            Node* next;

            template <typename... Args>
            Node(Node* n, Args&&... args) 
                : value(mystl::forward<Args>(args)...)
                , next(n) 
            {
            }
        };

        Node** buckets_            = nullptr;
        size_type bucket_count_    = 0;
        size_type size_            = 0;
        float     max_load_factor_ = 1.0f;
        hasher    hash_func_;
        key_equal equal_func_;

        size_type get_bucket_index(const Key& key, size_type b_count) const 
        {
            return hash_func_(key) % b_count;
        }

    public:
        // ========================================================================
        // ITERATORS (Separate for non-const and const access)
        // ========================================================================
        class Iterator 
        {
            friend class UnorderedMultiMap;
        private:
            Node* node_;
            size_type bucket_idx_;
            const UnorderedMultiMap* map_;

            Iterator(Node* node, size_type b_idx, const UnorderedMultiMap* map)
                : node_(node)
                , bucket_idx_(b_idx)
                , map_(map) 
            {
            }

        public:
            reference operator*() const { return node_->value; }
            value_type* operator->() const { return &(node_->value); }

            Iterator& operator++() 
            {
                node_ = node_->next;
                if (!node_) 
                {
                    while (++bucket_idx_ < map_->bucket_count_) 
                    {
                        if (map_->buckets_[bucket_idx_]) 
                        {
                            node_ = map_->buckets_[bucket_idx_];
                            break;
                        }
                    }
                }
                return *this;
            }

            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            bool operator==(const Iterator& other) const { return node_ == other.node_; }
            bool operator!=(const Iterator& other) const { return node_ != other.node_; }
        };

        class ConstIterator 
        {
            friend class UnorderedMultiMap;
        private:
            const Node* node_;
            size_type bucket_idx_;
            const UnorderedMultiMap* map_;

            ConstIterator(const Node* node, size_type b_idx, const UnorderedMultiMap* map)
                : node_(node)
                , bucket_idx_(b_idx)
                , map_(map) 
            {
            }

        public:
            const_reference operator*() const { return node_->value; }
            const value_type* operator->() const { return &(node_->value); }

            ConstIterator& operator++() 
            {
                node_ = node_->next;
                if (!node_) 
                {
                    while (++bucket_idx_ < map_->bucket_count_) 
                    {
                        if (map_->buckets_[bucket_idx_]) 
                        {
                            node_ = map_->buckets_[bucket_idx_];
                            break;
                        }
                    }
                }
                return *this;
            }

            ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
            bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
            bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }
        };

        using iterator = Iterator;
        using const_iterator = ConstIterator;

        iterator begin() noexcept 
        {
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                if (buckets_[i]) 
                    return iterator(buckets_[i], i, this);
            }
            return end();
        }
        iterator end() noexcept { return iterator(nullptr, bucket_count_, this); }

        const_iterator begin() const noexcept { return cbegin(); }
        const_iterator end() const noexcept { return cend(); }
        const_iterator cbegin() const noexcept 
        {
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                if (buckets_[i]) 
                    return const_iterator(buckets_[i], i, this);
            }
            return cend();
        }
        const_iterator cend() const noexcept { return const_iterator(nullptr, bucket_count_, this); }

    private:
        iterator make_iterator_safe(Node* node, size_type idx) 
        {
            if (node) 
                return iterator(node, idx, this);
            for (size_type i = idx + 1; i < bucket_count_; ++i) 
            {
                if (buckets_[i]) 
                    return iterator(buckets_[i], i, this);
            }
            return end();
        }

        const_iterator make_const_iterator_safe(const Node* node, size_type idx) const 
        {
            if (node) 
                return const_iterator(node, idx, this);
            for (size_type i = idx + 1; i < bucket_count_; ++i) 
            {
                if (buckets_[i]) 
                    return const_iterator(buckets_[i], i, this);
            }
            return cend();
        }

    public:
        // ========================================================================
        // RULE OF FIVE
        // ========================================================================
        UnorderedMultiMap() : UnorderedMultiMap(8) {}
        
        explicit UnorderedMultiMap(size_type bucket_count, const hasher& hf = hasher(), const key_equal& eql = key_equal())
            : bucket_count_(bucket_count)
            , hash_func_(hf)
            , equal_func_(eql) 
        {
            buckets_ = new Node*[bucket_count_](); 
        }

        ~UnorderedMultiMap() 
        { 
            clear(); 
            delete[] buckets_; 
        }

        UnorderedMultiMap(const UnorderedMultiMap& other)
            : bucket_count_(other.bucket_count_)
            , max_load_factor_(other.max_load_factor_)
            , hash_func_(other.hash_func_)
            , equal_func_(other.equal_func_) 
        {
            buckets_ = new Node*[bucket_count_]();
            for (size_type i = 0; i < other.bucket_count_; ++i) 
            {
                Node* curr = other.buckets_[i];
                Node* prev = nullptr;
                while (curr) 
                {
                    Node* new_node = new Node(nullptr, curr->value);
                    if (!prev) 
                        buckets_[i] = new_node;
                    else 
                        prev->next = new_node;
                    prev = new_node; curr = curr->next; size_++;
                }
            }
        }

        UnorderedMultiMap(UnorderedMultiMap&& other) noexcept
            : buckets_(other.buckets_)
            , bucket_count_(other.bucket_count_)
            , size_(other.size_)
            , max_load_factor_(other.max_load_factor_)
            , hash_func_(mystl::move(other.hash_func_))
            , equal_func_(mystl::move(other.equal_func_)) 
        {
            other.buckets_ = nullptr; other.bucket_count_ = 0; other.size_ = 0;
        }

        UnorderedMultiMap& operator=(const UnorderedMultiMap& other) 
        {
            if (this != &other) 
            { 
                UnorderedMultiMap tmp(other); 
                swap(tmp); 
            }
            return *this;
        }

        UnorderedMultiMap& operator=(UnorderedMultiMap&& other) noexcept 
        {
            if (this != &other) 
            {
                clear(); 
                delete[] buckets_;
                buckets_ = other.buckets_; 
                bucket_count_ = other.bucket_count_; 
                size_ = other.size_;
                max_load_factor_ = other.max_load_factor_;
                other.buckets_ = nullptr; 
                other.bucket_count_ = 0; 
                other.size_ = 0;
            }
            return *this;
        }

        // ========================================================================
        // SIZE AND CLEARING
        // ========================================================================
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
        [[nodiscard]] size_type size() const noexcept { return size_; }
        [[nodiscard]] float load_factor() const noexcept { return bucket_count_ == 0 ? 0.0f : static_cast<float>(size_) / bucket_count_; }
        
        void clear() noexcept 
        {
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                Node* curr = buckets_[i];
                while (curr) 
                { 
                    Node* next = curr->next; 
                    delete curr; 
                    curr = next; 
                }
                buckets_[i] = nullptr;
            }
            size_ = 0;
        }

        // ========================================================================
        // SEARCH AND RANGES
        // ========================================================================
        iterator find(const Key& key) 
        {
            if (bucket_count_ == 0) 
                return end();
            size_type idx = get_bucket_index(key, bucket_count_);
            Node* curr = buckets_[idx];
            while (curr) 
            {
                if (equal_func_(curr->value.first, key)) 
                    return iterator(curr, idx, this);
                curr = curr->next;
            }
            return end();
        }

        const_iterator find(const Key& key) const 
        {
            if (bucket_count_ == 0) 
                return cend();
            size_type idx = get_bucket_index(key, bucket_count_);
            const Node* curr = buckets_[idx];
            while (curr) 
            {
                if (equal_func_(curr->value.first, key)) 
                    return const_iterator(curr, idx, this);
                curr = curr->next;
            }
            return cend();
        }

        size_type count(const Key& key) const 
        {
            if (bucket_count_ == 0) 
                return 0;
            size_type idx = get_bucket_index(key, bucket_count_);
            const Node* curr = buckets_[idx];
            size_type cnt = 0;
            while (curr) 
            {
                if (equal_func_(curr->value.first, key)) 
                    cnt++;
                else if (cnt > 0) 
                    break;
                curr = curr->next;
            }
            return cnt;
        }

        mystl::Pair<iterator, iterator> equal_range(const Key& key) 
        {
            if (bucket_count_ == 0) 
                return { end(), end() };
            size_type idx = get_bucket_index(key, bucket_count_);
            Node* curr = buckets_[idx];

            while (curr) 
            {
                if (equal_func_(curr->value.first, key)) 
                {
                    Node* first = curr;
                    while (curr && equal_func_(curr->value.first, key)) 
                    {
                        curr = curr->next;
                    }
                    return { iterator(first, idx, this), make_iterator_safe(curr, idx) };
                }
                curr = curr->next;
            }
            return { end(), end() };
        }

        mystl::Pair<const_iterator, const_iterator> equal_range(const Key& key) const 
        {
            if (bucket_count_ == 0) 
                return { cend(), cend() };
            size_type idx = get_bucket_index(key, bucket_count_);
            const Node* curr = buckets_[idx];

            while (curr) 
            {
                if (equal_func_(curr->value.first, key)) 
                {
                    const Node* first = curr;
                    while (curr && equal_func_(curr->value.first, key)) 
                    {
                        curr = curr->next;
                    }
                    return { const_iterator(first, idx, this), make_const_iterator_safe(curr, idx) };
                }
                curr = curr->next;
            }
            return { cend(), cend() };
        }

        // ========================================================================
        // MODIFIERS
        // ========================================================================
        void rehash(size_type new_count) 
        {
            if (new_count <= bucket_count_) 
                return;
            Node** new_buckets = new Node*[new_count]();
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                Node* curr = buckets_[i];
                while (curr) 
                {
                    Node* next_node = curr->next;
                    size_type new_idx = get_bucket_index(curr->value.first, new_count);
                    curr->next = new_buckets[new_idx];
                    new_buckets[new_idx] = curr;
                    curr = next_node;
                }
            }
            delete[] buckets_; 
            buckets_ = new_buckets; 
            bucket_count_ = new_count;
        }

        template <typename... Args>
        iterator emplace(Args&&... args) 
        {
            Node* new_node = new Node(nullptr, mystl::forward<Args>(args)...);
            const Key& key = new_node->value.first;

            if (load_factor() + 1.0f / (bucket_count_ == 0 ? 1 : bucket_count_) > max_load_factor_) 
            {
                rehash(bucket_count_ == 0 ? 8 : bucket_count_ * 2);
            }

            size_type idx = get_bucket_index(key, bucket_count_);
            Node* curr = buckets_[idx];
            bool inserted = false;
            
            // Ensure duplicates with the same key remain adjacent
            if (curr && equal_func_(curr->value.first, key)) 
            {
                new_node->next = curr->next;
                curr->next = new_node;
                inserted = true;
            } 
            else 
            {
                while (curr && curr->next) 
                {
                    if (equal_func_(curr->next->value.first, key)) 
                    {
                        new_node->next = curr->next->next;
                        curr->next->next = new_node;
                        inserted = true;
                        break;
                    }
                    curr = curr->next;
                }
            }

            if (!inserted) 
            {
                new_node->next = buckets_[idx];
                buckets_[idx] = new_node;
            }

            size_++;
            return iterator(new_node, idx, this);
        }

        iterator insert(const value_type& value) { return emplace(value); }
        iterator insert(value_type&& value) { return emplace(mystl::move(value)); }

        size_type erase(const Key& key) 
        {
            if (bucket_count_ == 0) 
                return 0;
            size_type idx = get_bucket_index(key, bucket_count_);
            Node* curr = buckets_[idx];
            Node* prev = nullptr;
            size_type erased_count = 0;

            while (curr) 
            {
                if (equal_func_(curr->value.first, key)) 
                {
                    Node* to_delete = curr;
                    if (!prev) 
                        buckets_[idx] = curr->next;
                    else 
                        prev->next = curr->next;
                    
                    curr = curr->next;
                    delete to_delete;
                    size_--; erased_count++;
                } 
                else 
                {
                    if (erased_count > 0) 
                        break;
                    prev = curr; curr = curr->next;
                }
            }
            return erased_count;
        }

        void swap(UnorderedMultiMap& other) noexcept 
        {
            mystl::swap(buckets_, other.buckets_);
            mystl::swap(bucket_count_, other.bucket_count_);
            mystl::swap(size_, other.size_);
            mystl::swap(max_load_factor_, other.max_load_factor_);
        }
    };
} // namespace mystl